/*
 * Flash Block Device Driver using blk-mq framework
 * 
 * This driver creates a block device backed by Serial NOR Flash
 * that can be formatted with ext4 and mounted.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/hdreg.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/sysfs.h>

#define DEVICE_NAME "flashblk"
#define KERNEL_SECTOR_SIZE 512

/* Device parameters */
#define MYBLK_SECTOR_SIZE 512
#define MYBLK_MINORS 1
#define RAM_DATA_SIZE (3 * 1024 * 1024)  /* 3MB RAM (read-write, at offset 0) */
#define FLASH_DATA_SIZE (512 * 1024)  /* 512KB Flash = 128 sectors × 4KB (read-write, at offset RAM_DATA_SIZE) */
#define FLASH_START_ADDR 0x000000  /* Flash physical address */
#define FLASH_SECTOR_SIZE 4096  /* Flash sector size for erase (4KB) */
#define FLASH_MAX_SECTORS 128  /* Maximum sector ID (0-127) */
#define MYBLK_TOTAL_SIZE (RAM_DATA_SIZE + FLASH_DATA_SIZE)  /* Total: 3.5MB = RAM + Flash */

/* Flash I2C parameters - modify these based on your hardware */
#define FLASH_I2C_BUS 4
#define FLASH_I2C_ADDR 0x11  /* Adjust to your sensor address */
#define I2C_SMBUS_BLOCK_MAX 32
#define FLASH_READ_RETRY 3

/* Flash sensor info structure - adapt to your sensor_info_t */
struct flash_sensor_info {
    int bus_num;
    uint8_t sensor_addr;
    /* Add other fields from your sensor_info_t as needed */
};

/* Device structure */
struct myblk_device {
    unsigned long size;              /* Device size in bytes */
    u8 *cache;                       /* Cache buffer for read/write */
    u8 *ram_buf;                     /* RAM区缓冲区 */
    struct mutex lock;               /* Mutex for flash I/O (can sleep) */
    struct gendisk *gd;              /* Generic disk structure */
    struct blk_mq_tag_set tag_set;   /* blk-mq tag set */
    struct request_queue *queue;     /* Request queue */
    struct flash_sensor_info flash_info; /* Flash hardware info */
    struct i2c_client *i2c_client;   /* I2C client for flash access */
    /* Sysfs interface for direct flash access */
    struct class *flash_class;       /* Device class for sysfs */
    struct device *flash_device;     /* Device for sysfs */
    uint32_t debug_addr;             /* Flash address for debug read */
    uint32_t debug_len;              /* Length for debug read */
    u8 *debug_buf;                   /* Buffer for debug read data */
};

static struct myblk_device *myblk_dev = NULL;
static int myblk_major = 0;

/*
 * Flash I2C helper functions
 * These are simplified versions - integrate with your actual I2C functions
 */
static int flash_i2c_read_retry(int bus_num, uint8_t addr,
    uint8_t *reg_addr, uint8_t reg_size, uint8_t *buf, uint8_t buf_size)
{
    struct i2c_adapter *adapter;
    struct i2c_msg msgs[2];
    int ret, retry;

    adapter = i2c_get_adapter(bus_num);
    if (!adapter) {
        printk(KERN_ERR "flashblk: Failed to get I2C adapter %d\n", bus_num);
        return -ENODEV;
    }

    /* Write register address */
    msgs[0].addr = addr;
    msgs[0].flags = 0;
    msgs[0].len = reg_size;
    msgs[0].buf = reg_addr;

    /* Read data */
    msgs[1].addr = addr;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = buf_size;
    msgs[1].buf = buf;

    for (retry = 0; retry < FLASH_READ_RETRY; retry++) {
        ret = i2c_transfer(adapter, msgs, 2);
        if (ret == 2) {
            i2c_put_adapter(adapter);
            return 0;
        }
        usleep_range(1000, 2000);
    }

    i2c_put_adapter(adapter);
    return -EIO;
}

static int flash_i2c_write_retry(int bus_num, uint8_t addr,
    uint8_t *reg_addr, uint8_t reg_size, uint8_t *buf, uint8_t buf_size)
{
    struct i2c_adapter *adapter;
    struct i2c_msg msg;
    uint8_t *write_buf;
    int ret, retry;

    adapter = i2c_get_adapter(bus_num);
    if (!adapter) {
        printk(KERN_ERR "flashblk: Failed to get I2C adapter %d\n", bus_num);
        return -ENODEV;
    }

    write_buf = kmalloc(reg_size + buf_size, GFP_KERNEL);
    if (!write_buf) {
        i2c_put_adapter(adapter);
        return -ENOMEM;
    }

    memcpy(write_buf, reg_addr, reg_size);
    memcpy(write_buf + reg_size, buf, buf_size);

    msg.addr = addr;
    msg.flags = 0;
    msg.len = reg_size + buf_size;
    msg.buf = write_buf;

    for (retry = 0; retry < FLASH_READ_RETRY; retry++) {
        ret = i2c_transfer(adapter, &msg, 1);
        if (ret == 1) {
            kfree(write_buf);
            i2c_put_adapter(adapter);
            return 0;
        }
        usleep_range(1000, 2000);
    }

    kfree(write_buf);
    i2c_put_adapter(adapter);
    return -EIO;
}

static int hb_vin_i2c_write_reg16_data8(int bus_num, uint8_t addr,
    uint16_t reg, uint8_t data)
{
    struct i2c_adapter *adapter;
    struct i2c_msg msg;
    uint8_t buf[3];
    int ret;

    adapter = i2c_get_adapter(bus_num);
    if (!adapter)
        return -ENODEV;

    buf[0] = (reg >> 8) & 0xFF;
    buf[1] = reg & 0xFF;
    buf[2] = data;

    msg.addr = addr;
    msg.flags = 0;
    msg.len = 3;
    msg.buf = buf;

    ret = i2c_transfer(adapter, &msg, 1);
    i2c_put_adapter(adapter);

    return (ret == 1) ? 0 : -EIO;
}

/*
 * Raw flash read operation - reads from absolute flash address
 * This is used by the sysfs interface for debug access
 */
static int flash_read_raw(struct myblk_device *dev, uint32_t flash_addr,
    uint8_t *data, uint32_t bytes)
{
    int ret = 0;
    uint8_t reg_addr[4] = {0};
    uint8_t reg_size = 0;
    uint8_t buf[I2C_SMBUS_BLOCK_MAX] = {0};
    uint8_t buf_size = 0;
    uint8_t flash_offset = 0;
    struct flash_sensor_info *sensor_info = &dev->flash_info;

    printk(KERN_DEBUG "flashblk: Reading %u bytes from flash addr 0x%06X\n",
           bytes, flash_addr);

    /* Serial NOR Flash access unlock request */
    ret = hb_vin_i2c_write_reg16_data8(sensor_info->bus_num,
        sensor_info->sensor_addr, 0xFFFF, 0xF4);
    if (ret < 0) {
        printk(KERN_ERR "flashblk: Flash unlock failed\n");
        return ret;
    }
    usleep_range(2000, 3000);

    /* Serial NOR Flash read request */
    ret = hb_vin_i2c_write_reg16_data8(sensor_info->bus_num,
        sensor_info->sensor_addr, 0xFFFF, 0xF7);
    if (ret < 0) {
        printk(KERN_ERR "flashblk: Flash read request failed\n");
        goto lock_flash;
    }
    usleep_range(2000, 3000);

    /* Serial NOR Flash Read Subcommand */
    reg_addr[0] = 0x80;
    reg_addr[1] = 0x00;
    reg_addr[2] = 0x01;
    reg_size = 3;

    /* Address settings - use absolute flash address */
    buf[0] = (flash_addr >> 24) & 0xFF;
    buf[1] = (flash_addr >> 16) & 0xFF;
    buf[2] = (flash_addr >> 8) & 0xFF;
    buf[3] = flash_addr & 0xFF;
    buf[4] = 0x5a; /* Execute subcommand */
    buf_size = 5;

    ret = flash_i2c_write_retry(sensor_info->bus_num, sensor_info->sensor_addr,
        reg_addr, reg_size, buf, buf_size);
    if (ret < 0) {
        printk(KERN_ERR "flashblk: Flash read subcommand failed\n");
        goto lock_flash;
    }
    usleep_range(10000, 11000);

    /* Buffer read request */
    while (bytes > 0) {
        memset(buf, 0, I2C_SMBUS_BLOCK_MAX);
        reg_size = 2;
        memset(reg_addr, 0, reg_size);
        reg_addr[0] = 0x00;
        reg_addr[1] = flash_offset;

        if (bytes > I2C_SMBUS_BLOCK_MAX - reg_size) {
            buf_size = I2C_SMBUS_BLOCK_MAX - reg_size;
        } else {
            buf_size = bytes;
        }

        ret = flash_i2c_read_retry(sensor_info->bus_num, sensor_info->sensor_addr,
            reg_addr, reg_size, buf, buf_size);
        if (ret < 0) {
            printk(KERN_ERR "flashblk: Read failed at flash_offset 0x%x\n", flash_offset);
            goto lock_flash;
        }

        memcpy(data + flash_offset, buf, buf_size);
        bytes -= buf_size;
        flash_offset += buf_size;
    }
    usleep_range(10000, 11000);

lock_flash:
    /* Serial NOR Flash access lock request */
    hb_vin_i2c_write_reg16_data8(sensor_info->bus_num,
        sensor_info->sensor_addr, 0xFFFF, 0xF5);
    usleep_range(2000, 3000);

    return ret;
}

/*
 * Flash erase sector operation - based on sensor_flash_sector implementation
 * sector_id: sector number to erase (0-127)
 */
static int flash_erase_sector(struct myblk_device *dev, uint32_t sector_id)
{
    int ret = 0;
    uint8_t reg_addr[2] = {0};
    uint8_t reg_size = 0;
    uint8_t buf[6] = {0};
    uint8_t buf_size = 0;
    struct flash_sensor_info *sensor_info = &dev->flash_info;

    if (sector_id >= FLASH_MAX_SECTORS) {
        printk(KERN_ERR "flashblk: Invalid sector_id %u (must be 0-%d)\n", sector_id, FLASH_MAX_SECTORS - 1);
        return -EINVAL;
    }

    printk(KERN_DEBUG "flashblk: Erasing flash sector %u\n", sector_id);

    /* Serial NOR Flash access unlock request */
    ret = hb_vin_i2c_write_reg16_data8(sensor_info->bus_num,
        sensor_info->sensor_addr, 0xFFFF, 0xF4);
    if (ret < 0) {
        printk(KERN_ERR "flashblk: Flash unlock failed\n");
        return ret;
    }
    usleep_range(2000, 3000);

    /* Serial NOR Flash access request */
    ret = hb_vin_i2c_write_reg16_data8(sensor_info->bus_num,
        sensor_info->sensor_addr, 0xFFFF, 0xF7);
    if (ret < 0) {
        printk(KERN_ERR "flashblk: Flash access request failed\n");
        goto lock_flash;
    }
    usleep_range(2000, 3000);

    /* Erase sector command */
    reg_addr[0] = 0x80;
    reg_addr[1] = 0x00;
    reg_size = 2;

    buf[0] = 0x03;  /* Erase command */
    buf[1] = 0x00;
    buf[2] = sector_id >> 4;
    buf[3] = (sector_id & 0xf) << 4;
    buf[4] = 0x00;
    buf[5] = 0x5a;  /* Execute subcommand */
    buf_size = 6;

    ret = flash_i2c_write_retry(sensor_info->bus_num, sensor_info->sensor_addr,
        reg_addr, reg_size, buf, buf_size);
    if (ret < 0) {
        printk(KERN_ERR "flashblk: Flash erase command failed\n");
        goto lock_flash;
    }
    usleep_range(50000, 52000);  /* Wait for erase to complete */

lock_flash:
    /* Serial NOR Flash access lock request */
    hb_vin_i2c_write_reg16_data8(sensor_info->bus_num,
        sensor_info->sensor_addr, 0xFFFF, 0xF5);
    usleep_range(2000, 3000);

    return ret;
}

/*
 * Flash write operation - based on sensor_flash_sector implementation
 * offset is relative to block device start (within FLASH region)
 * This implementation writes data to flash using sector-based operations
 */
static int flash_write(struct myblk_device *dev, loff_t offset, const uint8_t *data, uint32_t bytes)
{
    int ret = 0;
    uint32_t flash_addr = FLASH_START_ADDR + offset;
    uint8_t reg_addr[4] = {0};
    uint8_t reg_size = 0;
    uint8_t buf[I2C_SMBUS_BLOCK_MAX] = {0};
    uint8_t buf_size = 0;
    uint8_t flash_offset = 0;
    uint32_t remaining = bytes;
    uint32_t data_offset = 0;
    struct flash_sensor_info *sensor_info = &dev->flash_info;

    printk(KERN_DEBUG "flashblk: Writing %u bytes to flash offset 0x%llx (addr 0x%06X)\n",
           bytes, offset, flash_addr);

    /* Serial NOR Flash access unlock request */
    ret = hb_vin_i2c_write_reg16_data8(sensor_info->bus_num,
        sensor_info->sensor_addr, 0xFFFF, 0xF4);
    if (ret < 0) {
        printk(KERN_ERR "flashblk: Flash unlock failed\n");
        return ret;
    }
    usleep_range(2000, 3000);

    /* Serial NOR Flash write access request */
    ret = hb_vin_i2c_write_reg16_data8(sensor_info->bus_num,
        sensor_info->sensor_addr, 0xFFFF, 0xF7);
    if (ret < 0) {
        printk(KERN_ERR "flashblk: Flash write access request failed\n");
        goto lock_flash;
    }
    usleep_range(2000, 3000);

    /* Write data in chunks to buffer, then program to flash */
    while (remaining > 0) {
        uint32_t chunk_size = (remaining > 16) ? 16 : remaining;
        
        /* Write data to buffer */
        reg_size = 2;
        reg_addr[0] = 0x00;
        reg_addr[1] = flash_offset;

        ret = flash_i2c_write_retry(sensor_info->bus_num, sensor_info->sensor_addr,
            reg_addr, reg_size, (uint8_t *)(data + data_offset), chunk_size);
        if (ret < 0) {
            printk(KERN_ERR "flashblk: Write to buffer failed at offset 0x%x\n", flash_offset);
            goto lock_flash;
        }

        data_offset += chunk_size;
        flash_offset += chunk_size;
        remaining -= chunk_size;

        /* If buffer is full (256 bytes) or no more data, program to flash */
        if (flash_offset >= 256 || remaining == 0) {
            /* Serial NOR Flash Write Subcommand */
            reg_addr[0] = 0x80;
            reg_addr[1] = 0x00;
            reg_size = 2;

            buf[0] = 0x02;  /* Write command */
            buf[1] = 0x00;
            buf[2] = (flash_addr >> 16) & 0xFF;
            buf[3] = (flash_addr >> 8) & 0xFF;
            buf[4] = flash_addr & 0xFF;
            buf[5] = 0x5a;  /* Execute subcommand */
            buf_size = 6;

            ret = flash_i2c_write_retry(sensor_info->bus_num, sensor_info->sensor_addr,
                reg_addr, reg_size, buf, buf_size);
            if (ret < 0) {
                printk(KERN_ERR "flashblk: Flash write subcommand failed\n");
                goto lock_flash;
            }
            usleep_range(3000, 4000);

            /* Move to next page */
            flash_addr += flash_offset;
            flash_offset = 0;
        }
    }

lock_flash:
    /* Serial NOR Flash access lock request */
    hb_vin_i2c_write_reg16_data8(sensor_info->bus_num,
        sensor_info->sensor_addr, 0xFFFF, 0xF5);
    usleep_range(2000, 3000);

    return ret;
}

/*
 * Flash write with automatic sector erase
 * This function erases the necessary sectors before writing
 * offset: offset within Flash region (relative to FLASH_START_ADDR)
 */
static int flash_write_with_erase(struct myblk_device *dev, loff_t offset, 
    const uint8_t *data, uint32_t bytes)
{
    int ret = 0;
    uint32_t start_sector = offset / FLASH_SECTOR_SIZE;
    uint32_t end_sector = (offset + bytes - 1) / FLASH_SECTOR_SIZE;
    uint32_t sector;

    printk(KERN_INFO "flashblk: Writing %u bytes to flash offset 0x%llx (sectors %u-%u)\n", 
           bytes, offset, start_sector, end_sector);

    /* Erase affected sectors first */
    for (sector = start_sector; sector <= end_sector; sector++) {
        ret = flash_erase_sector(dev, sector);
        if (ret < 0) {
            printk(KERN_ERR "flashblk: Failed to erase sector %u\n", sector);
            return ret;
        }
    }

    /* Now write the data using the existing flash_write function */
    ret = flash_write(dev, offset, data, bytes);
    
    return ret;
}

/*
 * Hybrid read operation with new layout: RAM first, then Flash
 * Device layout: [RAM: 0 - RAM_DATA_SIZE] [Flash: RAM_DATA_SIZE - MYBLK_TOTAL_SIZE]
 */
static int hybrid_read(struct myblk_device *dev, loff_t offset,
    uint8_t *data, uint32_t bytes)
{
    int ret = 0;
    
    if (offset < RAM_DATA_SIZE) {
        /* Read from RAM region */
        uint32_t ram_bytes = bytes;
        if (offset + bytes > RAM_DATA_SIZE) {
            ram_bytes = RAM_DATA_SIZE - offset;
        }
        memcpy(data, dev->ram_buf + offset, ram_bytes);
        
        /* If read spans into Flash region, read Flash part too */
        if (ram_bytes < bytes) {
            uint32_t flash_offset = 0;
            uint32_t flash_bytes = bytes - ram_bytes;
            uint32_t flash_addr = FLASH_START_ADDR + flash_offset;
            
            ret = flash_read_raw(dev, flash_addr, data + ram_bytes, flash_bytes);
            if (ret < 0) return ret;
        }
    } else {
        /* Read from Flash region */
        uint32_t flash_offset = offset - RAM_DATA_SIZE;
        uint32_t flash_addr = FLASH_START_ADDR + flash_offset;
        
        if (flash_offset + bytes > FLASH_DATA_SIZE) {
            bytes = FLASH_DATA_SIZE - flash_offset;
        }
        ret = flash_read_raw(dev, flash_addr, data, bytes);
    }
    return ret;
}

/*
 * Hybrid write operation with new layout: RAM first (writable), then Flash (writable with erase)
 * Device layout: [RAM: 0 - RAM_DATA_SIZE] [Flash: RAM_DATA_SIZE - MYBLK_TOTAL_SIZE]
 */
static int hybrid_write(struct myblk_device *dev, loff_t offset,
    const uint8_t *data, uint32_t bytes)
{
    int ret = 0;
    
    if (offset < RAM_DATA_SIZE) {
        /* Write to RAM region */
        uint32_t ram_bytes = bytes;
        if (offset + bytes > RAM_DATA_SIZE) {
            /* Write spans both RAM and Flash regions */
            ram_bytes = RAM_DATA_SIZE - offset;
            printk(KERN_DEBUG "flashblk: Write spans RAM and Flash regions\n");
        }
        memcpy(dev->ram_buf + offset, data, ram_bytes);
        
        /* If write continues into Flash region */
        if (ram_bytes < bytes) {
            uint32_t flash_offset = 0;
            uint32_t flash_bytes = bytes - ram_bytes;
            ret = flash_write_with_erase(dev, flash_offset, data + ram_bytes, flash_bytes);
            if (ret < 0) {
                printk(KERN_ERR "flashblk: Flash write failed\n");
                return ret;
            }
        }
        return 0;
    } else {
        /* Write to Flash region */
        uint32_t flash_offset = offset - RAM_DATA_SIZE;
        if (flash_offset + bytes > FLASH_DATA_SIZE) {
            printk(KERN_WARNING "flashblk: Write beyond Flash region\n");
            bytes = FLASH_DATA_SIZE - flash_offset;
        }
        ret = flash_write_with_erase(dev, flash_offset, data, bytes);
        return ret;
    }
}

/*
 * Handle an I/O request - Flash backend implementation
 */
static blk_status_t myblk_request(struct blk_mq_hw_ctx *hctx,
                                   const struct blk_mq_queue_data *bd)
{
    struct request *req = bd->rq;
    struct myblk_device *dev = req->q->queuedata;
    struct bio_vec bvec;
    struct req_iterator iter;
    loff_t pos;
    loff_t dev_size;
    blk_status_t ret = BLK_STS_OK;
    int io_ret;
    u8 *temp_buf = NULL;
    size_t total_len = blk_rq_bytes(req);

    blk_mq_start_request(req);

    pos = blk_rq_pos(req) * MYBLK_SECTOR_SIZE;
    dev_size = dev->size;

    temp_buf = kmalloc(total_len, GFP_KERNEL);
    if (!temp_buf) {
        printk(KERN_ERR "flashblk: Failed to allocate temp buffer\n");
        ret = BLK_STS_RESOURCE;
        goto out;
    }

    if (req_op(req) == REQ_OP_WRITE) {
        size_t offset = 0;
        rq_for_each_segment(bvec, req, iter) {
            void *src = kmap_atomic(bvec.bv_page);
            memcpy(temp_buf + offset, src + bvec.bv_offset, bvec.bv_len);
            kunmap_atomic(src);
            offset += bvec.bv_len;
        }
    }

    if (pos + total_len > dev_size) {
        printk(KERN_ERR "flashblk: Request beyond device size\n");
        ret = BLK_STS_IOERR;
        goto free_buf;
    }

    mutex_lock(&dev->lock);

    switch (req_op(req)) {
    case REQ_OP_READ:
        io_ret = hybrid_read(dev, pos, temp_buf, total_len);
        if (io_ret < 0) {
            printk(KERN_ERR "flashblk: hybrid read failed at 0x%llx\n", pos);
            ret = BLK_STS_IOERR;
        }
        break;
    case REQ_OP_WRITE:
        io_ret = hybrid_write(dev, pos, temp_buf, total_len);
        if (io_ret < 0) {
            printk(KERN_WARNING "flashblk: hybrid write failed at 0x%llx\n", pos);
            ret = BLK_STS_IOERR;
        }
        break;
    default:
        printk(KERN_WARNING "flashblk: Unsupported request operation\n");
        ret = BLK_STS_NOTSUPP;
        break;
    }

    mutex_unlock(&dev->lock);

    if (ret == BLK_STS_OK && req_op(req) == REQ_OP_READ) {
        size_t offset = 0;
        rq_for_each_segment(bvec, req, iter) {
            void *dst = kmap_atomic(bvec.bv_page);
            memcpy(dst + bvec.bv_offset, temp_buf + offset, bvec.bv_len);
            kunmap_atomic(dst);
            offset += bvec.bv_len;
        }
    }

free_buf:
    kfree(temp_buf);
out:
    blk_mq_end_request(req, ret);
    return ret;
}
/*
 * Block device operations
 */
static int myblk_open(struct block_device *bdev, fmode_t mode)
{
    printk(KERN_INFO "myblkdev: Device opened\n");
    return 0;
}

static void myblk_release(struct gendisk *gd, fmode_t mode)
{
    printk(KERN_INFO "myblkdev: Device closed\n");
}

static int myblk_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
    /* 
     * Return fake geometry for compatibility 
     * heads = 4, sectors = 16
     */
    struct myblk_device *dev = bdev->bd_disk->private_data;
    
    geo->heads = 4;
    geo->sectors = 16;
    geo->cylinders = (dev->size / MYBLK_SECTOR_SIZE) / (geo->heads * geo->sectors);
    geo->start = 0;
    
    return 0;
}

static const struct block_device_operations myblk_fops = {
    .owner = THIS_MODULE,
    .open = myblk_open,
    .release = myblk_release,
    .getgeo = myblk_getgeo,
};

/*
 * blk-mq operations
 */
static struct blk_mq_ops myblk_mq_ops __maybe_unused = {
    .queue_rq = myblk_request,
};

/*
 * Sysfs interface for direct flash access
 */

static struct attribute *flashblk_attrs[] = {
    NULL,
};
static const struct attribute_group flashblk_attr_group = {
    .attrs = flashblk_attrs,
};

/* Show flash address */
static ssize_t flash_addr_show(struct device *dev,
    struct device_attribute *attr, char *buf)
    __attribute__((unused));

static ssize_t flash_addr_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    struct myblk_device *mydev = dev_get_drvdata(dev);
    return sprintf(buf, "0x%08X\n", mydev->debug_addr);
}

/* Store flash address */
static ssize_t flash_addr_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
    __attribute__((unused));

static ssize_t flash_addr_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
{
    struct myblk_device *mydev = dev_get_drvdata(dev);
    uint32_t addr;
    if (kstrtou32(buf, 0, &addr) != 0)
        return -EINVAL;
    mydev->debug_addr = addr;
    printk(KERN_INFO "flashblk: Debug address set to 0x%08X\n", addr);
    return count;
}

/*
 * Initialize the device
 */
static int __init myblk_init(void)
{
    int ret = 0;

    printk(KERN_INFO "flashblk: Initializing Flash+RAM hybrid block device driver\n");

    /* Allocate device structure */
    myblk_dev = kzalloc(sizeof(struct myblk_device), GFP_KERNEL);
    if (!myblk_dev) {
        printk(KERN_ERR "flashblk: Failed to allocate device structure\n");
        return -ENOMEM;
    }

    /* Set device size */
    myblk_dev->size = MYBLK_TOTAL_SIZE;

    /* Allocate cache buffer */
    myblk_dev->cache = vmalloc(MYBLK_SECTOR_SIZE);
    if (!myblk_dev->cache) {
        printk(KERN_ERR "flashblk: Failed to allocate cache buffer\n");
        ret = -ENOMEM;
        goto out_free_dev;
    }

    /* Allocate RAM buffer */
    myblk_dev->ram_buf = vmalloc(RAM_DATA_SIZE);
    if (!myblk_dev->ram_buf) {
        printk(KERN_ERR "flashblk: Failed to allocate RAM buffer\n");
        ret = -ENOMEM;
        goto out_free_cache;
    }
    memset(myblk_dev->ram_buf, 0, RAM_DATA_SIZE);

    /* Initialize flash info */
    myblk_dev->flash_info.bus_num = FLASH_I2C_BUS;
    myblk_dev->flash_info.sensor_addr = FLASH_I2C_ADDR;

    /* Initialize mutex */
    mutex_init(&myblk_dev->lock);

    /* Register block device */
    myblk_major = register_blkdev(0, DEVICE_NAME);
    if (myblk_major < 0) {
        printk(KERN_ERR "flashblk: Failed to register block device\n");
        ret = myblk_major;
        goto out_free_ram;
    }

    /* Initialize blk-mq tag set */
    myblk_dev->tag_set.ops = &myblk_mq_ops;
    myblk_dev->tag_set.nr_hw_queues = 1;
    myblk_dev->tag_set.queue_depth = 128;
    myblk_dev->tag_set.numa_node = NUMA_NO_NODE;
    myblk_dev->tag_set.cmd_size = 0;
    myblk_dev->tag_set.flags = BLK_MQ_F_SHOULD_MERGE | BLK_MQ_F_BLOCKING;
    myblk_dev->tag_set.driver_data = myblk_dev;

    ret = blk_mq_alloc_tag_set(&myblk_dev->tag_set);
    if (ret) {
        printk(KERN_ERR "flashblk: Failed to allocate tag set\n");
        goto out_unregister;
    }

    /* Allocate disk */
    myblk_dev->gd = blk_mq_alloc_disk(&myblk_dev->tag_set, myblk_dev);
    if (IS_ERR(myblk_dev->gd)) {
        ret = PTR_ERR(myblk_dev->gd);
        printk(KERN_ERR "flashblk: Failed to allocate disk\n");
        goto out_free_tag_set;
    }

    myblk_dev->gd->major = myblk_major;
    myblk_dev->gd->first_minor = 0;
    myblk_dev->gd->minors = MYBLK_MINORS;
    myblk_dev->gd->fops = &myblk_fops;
    myblk_dev->gd->private_data = myblk_dev;
    snprintf(myblk_dev->gd->disk_name, 32, DEVICE_NAME);

    myblk_dev->queue = myblk_dev->gd->queue;
    myblk_dev->queue->queuedata = myblk_dev;
    blk_queue_logical_block_size(myblk_dev->queue, MYBLK_SECTOR_SIZE);
    blk_queue_physical_block_size(myblk_dev->queue, MYBLK_SECTOR_SIZE);
    
    /* Set capacity AFTER setting block sizes */
    set_capacity(myblk_dev->gd, myblk_dev->size / MYBLK_SECTOR_SIZE);

    /* Add disk */
    ret = add_disk(myblk_dev->gd);
    if (ret) {
        printk(KERN_ERR "flashblk: Failed to add disk\n");
        goto out_cleanup_disk;
    }

    printk(KERN_INFO "flashblk: Flash+RAM hybrid block device initialized successfully\n");
    printk(KERN_INFO "flashblk: Device size: %lu bytes (%lu sectors)\n",
           myblk_dev->size, myblk_dev->size / MYBLK_SECTOR_SIZE);
    printk(KERN_INFO "flashblk: RAM region (read-write): 0 - %d bytes\n", RAM_DATA_SIZE);
    printk(KERN_INFO "flashblk: Flash region (read-write with erase): %d - %lu bytes\n", RAM_DATA_SIZE, myblk_dev->size);
    printk(KERN_INFO "flashblk: Flash sector size: %d bytes\n", FLASH_SECTOR_SIZE);

    return 0;

out_cleanup_disk:
    put_disk(myblk_dev->gd);
out_free_tag_set:
    blk_mq_free_tag_set(&myblk_dev->tag_set);
out_unregister:
    unregister_blkdev(myblk_major, DEVICE_NAME);
out_free_ram:
    vfree(myblk_dev->ram_buf);
out_free_cache:
    vfree(myblk_dev->cache);
out_free_dev:
    kfree(myblk_dev);
    myblk_dev = NULL;
    return ret;
}

/*
 * Cleanup the device
 */
static void __exit myblk_exit(void)
{
    printk(KERN_INFO "flashblk: Cleaning up Flash+RAM hybrid block device driver\n");

    if (myblk_dev) {
        if (myblk_dev->flash_device) {
            sysfs_remove_group(&myblk_dev->flash_device->kobj, &flashblk_attr_group);
            device_destroy(myblk_dev->flash_class, MKDEV(0, 0));
        }
        if (myblk_dev->flash_class) {
            class_destroy(myblk_dev->flash_class);
        }
        if (myblk_dev->debug_buf) {
            kfree(myblk_dev->debug_buf);
        }
        if (myblk_dev->gd) {
            del_gendisk(myblk_dev->gd);
            put_disk(myblk_dev->gd);
        }
        blk_mq_free_tag_set(&myblk_dev->tag_set);
        if (myblk_major > 0)
            unregister_blkdev(myblk_major, DEVICE_NAME);
        if (myblk_dev->cache)
            vfree(myblk_dev->cache);
        if (myblk_dev->ram_buf)
            vfree(myblk_dev->ram_buf);
        kfree(myblk_dev);
    }

    printk(KERN_INFO "flashblk: Hybrid block device driver unloaded\n");
}

module_init(myblk_init);
module_exit(myblk_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huaizhen.lv");
MODULE_DESCRIPTION("Serial NOR Flash Block Device Driver with blk-mq");
MODULE_VERSION("1.0");
