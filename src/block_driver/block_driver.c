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

/* Device parameters - 256 bytes starting at 0x0C0000 */
#define MYBLK_SECTOR_SIZE 512
#define MYBLK_NSECTORS 1  /* Only 1 sector (512 bytes, but only 256 bytes valid) */
#define MYBLK_MINORS 1
#define FLASH_DATA_SIZE 786813  /* Actual flash data size */
#define FLASH_START_ADDR 0x000000  /* Flash start address */

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
 * Flash read operation - reads 256 bytes from 0x0C0000
 * offset is relative to the block device (0-255 for our 256 byte device)
 */
static int flash_read(struct myblk_device *dev, loff_t offset,
    uint8_t *data, uint32_t bytes)
{
    int ret = 0;
    uint32_t flash_addr;

    /* Limit read to available flash data */
    if (offset >= FLASH_DATA_SIZE) {
        memset(data, 0, bytes);
        return 0;
    }
    if (offset + bytes > FLASH_DATA_SIZE) {
        bytes = FLASH_DATA_SIZE - offset;
        /* Zero out the rest */
        if (offset + bytes < MYBLK_SECTOR_SIZE) {
            memset(data + bytes, 0, MYBLK_SECTOR_SIZE - offset - bytes);
        }
    }

    /* Calculate actual flash address */
    flash_addr = FLASH_START_ADDR + offset;

    printk(KERN_DEBUG "flashblk: Reading %u bytes from flash addr 0x%06X (offset 0x%llX)\n",
           bytes, flash_addr, offset);

    /* Use the raw flash read function */
    ret = flash_read_raw(dev, flash_addr, data, bytes);
    return ret;
}

/*
 * Flash write operation - you need to implement this based on your flash chip
 */
static int flash_write(struct myblk_device *dev, loff_t offset,
    const uint8_t *data, uint32_t bytes)
{
    /* 
     * TODO: Implement flash write operation
     * This depends on your flash chip's write protocol
     * May need sector erase before write
     */
    printk(KERN_WARNING "flashblk: Flash write not implemented yet\n");
    return -ENOSYS;
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
    int flash_ret;
    u8 *temp_buf = NULL;
    size_t total_len = blk_rq_bytes(req);

    /* Start request processing */
    blk_mq_start_request(req);

    /* Get position and device size */
    pos = blk_rq_pos(req) * MYBLK_SECTOR_SIZE;
    dev_size = dev->size;

    /* Allocate temporary buffer for the entire request */
    temp_buf = kmalloc(total_len, GFP_KERNEL);
    if (!temp_buf) {
        printk(KERN_ERR "flashblk: Failed to allocate temp buffer\n");
        ret = BLK_STS_RESOURCE;
        goto done;
    }

    /* Handle WRITE: copy data from bio to temp buffer first */
    if (req_op(req) == REQ_OP_WRITE) {
        size_t offset = 0;
        rq_for_each_segment(bvec, req, iter) {
            void *src = kmap_atomic(bvec.bv_page);
            memcpy(temp_buf + offset, src + bvec.bv_offset, bvec.bv_len);
            kunmap_atomic(src);
            offset += bvec.bv_len;
        }
    }

    /* Check boundaries */
    if (pos + total_len > dev_size) {
        printk(KERN_ERR "flashblk: Request beyond device size\n");
        ret = BLK_STS_IOERR;
        goto free_buf;
    }

    /* Lock for flash access and perform I/O (can sleep here) */
    mutex_lock(&dev->lock);

    switch (req_op(req)) {
    case REQ_OP_READ:
        /* Read from Flash to temp buffer */
        flash_ret = flash_read(dev, pos, temp_buf, total_len);
        if (flash_ret < 0) {
            printk(KERN_ERR "flashblk: Flash read failed at 0x%llx\n", pos);
            ret = BLK_STS_IOERR;
        }
        break;
    case REQ_OP_WRITE:
        /* Write from temp buffer to Flash */
        flash_ret = flash_write(dev, pos, temp_buf, total_len);
        if (flash_ret < 0) {
            printk(KERN_WARNING "flashblk: Flash write failed at 0x%llx\n", pos);
            ret = BLK_STS_IOERR;
        }
        break;
    default:
        printk(KERN_WARNING "flashblk: Unsupported request operation\n");
        ret = BLK_STS_NOTSUPP;
        break;
    }

    mutex_unlock(&dev->lock);

    /* Handle READ: copy data from temp buffer to bio */
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

done:
    /* Complete the request */
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
static struct blk_mq_ops myblk_mq_ops = {
    .queue_rq = myblk_request,
};

/*
 * Sysfs interface for direct flash access
 */

/* Show flash address */
static ssize_t flash_addr_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    struct myblk_device *mydev = dev_get_drvdata(dev);
    return sprintf(buf, "0x%08X\n", mydev->debug_addr);
}

/* Store flash address */
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

/* Show flash length */
static ssize_t flash_len_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    struct myblk_device *mydev = dev_get_drvdata(dev);
    return sprintf(buf, "%u\n", mydev->debug_len);
}

/* Store flash length */
static ssize_t flash_len_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
{
    struct myblk_device *mydev = dev_get_drvdata(dev);
    uint32_t len;
    
    if (kstrtou32(buf, 0, &len) != 0)
        return -EINVAL;
    
    if (len > 256) {
        printk(KERN_WARNING "flashblk: Length limited to 256 bytes\n");
        len = 256;
    }
    
    mydev->debug_len = len;
    printk(KERN_INFO "flashblk: Debug length set to %u\n", len);
    return count;
}

/* Read flash data - trigger actual flash read */
static ssize_t flash_data_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    struct myblk_device *mydev = dev_get_drvdata(dev);
    int ret;
    int i;
    ssize_t len = 0;
    
    if (mydev->debug_len == 0 || mydev->debug_len > 256) {
        return sprintf(buf, "Error: Invalid length (must be 1-256)\n");
    }
    
    if (!mydev->debug_buf) {
        mydev->debug_buf = kmalloc(256, GFP_KERNEL);
        if (!mydev->debug_buf)
            return sprintf(buf, "Error: Memory allocation failed\n");
    }
    
    memset(mydev->debug_buf, 0, 256);
    
    /* Perform flash read */
    mutex_lock(&mydev->lock);
    ret = flash_read_raw(mydev, mydev->debug_addr, mydev->debug_buf, mydev->debug_len);
    mutex_unlock(&mydev->lock);
    
    if (ret < 0) {
        return sprintf(buf, "Error: Flash read failed (ret=%d)\n", ret);
    }
    
    for (i = 0; i < mydev->debug_len; i++) {
        if (i % 16 == 0)
            len += sprintf(buf + len, "\n%04X: ", i);
        len += sprintf(buf + len, "%02X ", mydev->debug_buf[i]);
    }
    len += sprintf(buf + len, "\n");
    
    return len;
}

/* Store flash_data - simplified interface: "addr len" */
static ssize_t flash_data_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
{
    struct myblk_device *mydev = dev_get_drvdata(dev);
    uint32_t addr, len;
    int ret;
    
    ret = sscanf(buf, "%i %u", &addr, &len);
    if (ret != 2) {
        printk(KERN_ERR "flashblk: Invalid format. Use: echo 0xADDRESS LENGTH > flash_data\n");
        return -EINVAL;
    }
    
    if (len > 256) {
        printk(KERN_WARNING "flashblk: Length limited to 256 bytes\n");
        len = 256;
    }
    
    mydev->debug_addr = addr;
    mydev->debug_len = len;
    
    printk(KERN_INFO "flashblk: Set debug addr=0x%08X len=%u\n", addr, len);
    return count;
}

/* Device attributes */
static DEVICE_ATTR(flash_addr, 0644, flash_addr_show, flash_addr_store);
static DEVICE_ATTR(flash_len, 0644, flash_len_show, flash_len_store);
static DEVICE_ATTR(flash_data, 0644, flash_data_show, flash_data_store);

static struct attribute *flashblk_attrs[] = {
    &dev_attr_flash_addr.attr,
    &dev_attr_flash_len.attr,
    &dev_attr_flash_data.attr,
    NULL,
};

static const struct attribute_group flashblk_attr_group = {
    .attrs = flashblk_attrs,
};

/*
 * Initialize the device
 */
static int __init myblk_init(void)
{
    int ret;

    printk(KERN_INFO "flashblk: Initializing Flash block device driver\n");

    /* Allocate device structure */
    myblk_dev = kzalloc(sizeof(struct myblk_device), GFP_KERNEL);
    if (!myblk_dev) {
        printk(KERN_ERR "flashblk: Failed to allocate device structure\n");
        return -ENOMEM;
    }

    /* Set device size - adjust to your flash capacity */
    myblk_dev->size = MYBLK_NSECTORS * MYBLK_SECTOR_SIZE;

    /* Initialize Flash sensor info - MODIFY THESE VALUES */
    myblk_dev->flash_info.bus_num = FLASH_I2C_BUS;
    myblk_dev->flash_info.sensor_addr = FLASH_I2C_ADDR;

    /* Allocate cache buffer for flash operations */
    myblk_dev->cache = vmalloc(MYBLK_SECTOR_SIZE * 16); /* 8KB cache */
    if (!myblk_dev->cache) {
        printk(KERN_ERR "flashblk: Failed to allocate cache buffer\n");
        ret = -ENOMEM;
        goto out_free_dev;
    }
    memset(myblk_dev->cache, 0, MYBLK_SECTOR_SIZE * 16);

    /* Initialize mutex (can sleep, suitable for I2C operations) */
    mutex_init(&myblk_dev->lock);

    /* Register block device */
    myblk_major = register_blkdev(0, DEVICE_NAME);
    if (myblk_major < 0) {
        printk(KERN_ERR "myblkdev: Failed to register block device\n");
        ret = myblk_major;
        goto out_free_data;
    }

    /* Initialize tag set for blk-mq */
    myblk_dev->tag_set.ops = &myblk_mq_ops;
    myblk_dev->tag_set.nr_hw_queues = 1;
    myblk_dev->tag_set.queue_depth = 128;
    myblk_dev->tag_set.numa_node = NUMA_NO_NODE;
    myblk_dev->tag_set.cmd_size = 0;
    myblk_dev->tag_set.flags = BLK_MQ_F_SHOULD_MERGE;
    myblk_dev->tag_set.driver_data = myblk_dev;

    ret = blk_mq_alloc_tag_set(&myblk_dev->tag_set);
    if (ret) {
        printk(KERN_ERR "myblkdev: Failed to allocate tag set\n");
        goto out_unregister;
    }

    /* Allocate disk */
    myblk_dev->gd = blk_mq_alloc_disk(&myblk_dev->tag_set, myblk_dev);
    if (IS_ERR(myblk_dev->gd)) {
        ret = PTR_ERR(myblk_dev->gd);
        printk(KERN_ERR "myblkdev: Failed to allocate disk\n");
        goto out_free_tag_set;
    }

    myblk_dev->queue = myblk_dev->gd->queue;
    myblk_dev->queue->queuedata = myblk_dev;

    /* Set up gendisk */
    myblk_dev->gd->major = myblk_major;
    myblk_dev->gd->first_minor = 0;
    myblk_dev->gd->minors = MYBLK_MINORS;
    myblk_dev->gd->fops = &myblk_fops;
    myblk_dev->gd->private_data = myblk_dev;
    snprintf(myblk_dev->gd->disk_name, 32, DEVICE_NAME);
    set_capacity(myblk_dev->gd, MYBLK_NSECTORS);

    /* Set logical block size */
    blk_queue_logical_block_size(myblk_dev->queue, MYBLK_SECTOR_SIZE);
    blk_queue_physical_block_size(myblk_dev->queue, MYBLK_SECTOR_SIZE);

    /* Add disk */
    ret = add_disk(myblk_dev->gd);
    if (ret) {
        printk(KERN_ERR "flashblk: Failed to add disk\n");
        goto out_cleanup_disk;
    }

    /* Create sysfs interface for debug access */
    myblk_dev->flash_class = class_create(THIS_MODULE, "flashblk");
    if (IS_ERR(myblk_dev->flash_class)) {
        printk(KERN_WARNING "flashblk: Failed to create class for sysfs\n");
        myblk_dev->flash_class = NULL;
    } else {
        myblk_dev->flash_device = device_create(myblk_dev->flash_class, NULL,
                                                 MKDEV(0, 0), myblk_dev, "flashblk");
        if (IS_ERR(myblk_dev->flash_device)) {
            printk(KERN_WARNING "flashblk: Failed to create device for sysfs\n");
            myblk_dev->flash_device = NULL;
        } else {
            ret = sysfs_create_group(&myblk_dev->flash_device->kobj, &flashblk_attr_group);
            if (ret) {
                printk(KERN_WARNING "flashblk: Failed to create sysfs group\n");
            } else {
                printk(KERN_INFO "flashblk: Sysfs interface created at /sys/class/flashblk/\n");
                printk(KERN_INFO "flashblk: Usage: echo 0xADDRESS LENGTH > /sys/class/flashblk/flash_data\n");
                printk(KERN_INFO "flashblk:        cat /sys/class/flashblk/flash_data\n");
            }
        }
    }

    /* Initialize debug parameters */
    myblk_dev->debug_addr = FLASH_START_ADDR;
    myblk_dev->debug_len = 1;
    myblk_dev->debug_buf = NULL;

    printk(KERN_INFO "flashblk: Block device registered successfully\n");
    printk(KERN_INFO "flashblk: Device: /dev/%s, Size: %u bytes (%u sectors)\n", 
           DEVICE_NAME, FLASH_DATA_SIZE, MYBLK_NSECTORS);
    printk(KERN_INFO "flashblk: Flash Address: 0x%06X - 0x%06X\n",
           FLASH_START_ADDR, FLASH_START_ADDR + FLASH_DATA_SIZE - 1);
    printk(KERN_INFO "flashblk: I2C Bus: %d, Address: 0x%02X\n",
           myblk_dev->flash_info.bus_num, myblk_dev->flash_info.sensor_addr);

    return 0;

out_cleanup_disk:
    put_disk(myblk_dev->gd);
out_free_tag_set:
    blk_mq_free_tag_set(&myblk_dev->tag_set);
out_unregister:
    unregister_blkdev(myblk_major, DEVICE_NAME);
out_free_data:
    vfree(myblk_dev->cache);
out_free_dev:
    kfree(myblk_dev);
    return ret;
}

/*
 * Cleanup the device
 */
static void __exit myblk_exit(void)
{
    printk(KERN_INFO "flashblk: Cleaning up Flash block device driver\n");

    if (myblk_dev) {
        /* Remove sysfs interface */
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
        
        kfree(myblk_dev);
    }

    printk(KERN_INFO "flashblk: Block device driver unloaded\n");
}

module_init(myblk_init);
module_exit(myblk_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huaizhen.lv");
MODULE_DESCRIPTION("Serial NOR Flash Block Device Driver with blk-mq");
MODULE_VERSION("1.0");
