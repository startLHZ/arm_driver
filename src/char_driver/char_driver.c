/*
 * char_driver.c - 虚拟字符设备驱动
 * 
 * 功能：
 * - 基于内存的虚拟字符设备
 * - 支持 open, read, write, llseek, ioctl
 * - 自动创建设备节点 /dev/mychardev
 * - 支持多个进程同时访问（带互斥锁保护）
 * - 支持 ioctl 命令（清除、获取大小等）
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "mychardev"
#define CLASS_NAME  "chardrv"
#define BUFFER_SIZE (4096)  /* 4KB 设备缓冲区 */

/* IOCTL 命令定义 */
#define CHARDEV_IOC_MAGIC  'k'
#define CHARDEV_IOCRESET    _IO(CHARDEV_IOC_MAGIC,  0)   /* 清除缓冲区 */
#define CHARDEV_IOCGETSIZE  _IOR(CHARDEV_IOC_MAGIC, 1, int)  /* 获取缓冲区大小 */
#define CHARDEV_IOCGETUSED  _IOR(CHARDEV_IOC_MAGIC, 2, int)  /* 获取已使用大小 */

/* 设备私有数据结构 */
struct char_device_data {
    char *buffer;           /* 设备缓冲区 */
    size_t buffer_size;     /* 缓冲区大小 */
    size_t data_size;       /* 当前数据大小 */
    struct mutex lock;      /* 互斥锁 */
    struct cdev cdev;       /* 字符设备结构 */
};

static struct char_device_data *char_dev_data;
static dev_t dev_num;               /* 设备号 */
static struct class *char_class;    /* 设备类 */
static struct device *char_device;  /* 设备 */

/*
 * 打开设备
 */
static int char_dev_open(struct inode *inode, struct file *filp)
{
    struct char_device_data *dev_data;
    
    dev_data = container_of(inode->i_cdev, struct char_device_data, cdev);
    filp->private_data = dev_data;
    
    pr_info("CharDev: Device opened by process %d (%s)\n", 
            current->pid, current->comm);
    
    return 0;
}

/*
 * 关闭设备
 */
static int char_dev_release(struct inode *inode, struct file *filp)
{
    pr_info("CharDev: Device closed by process %d (%s)\n",
            current->pid, current->comm);
    
    return 0;
}

/*
 * 读取设备
 */
static ssize_t char_dev_read(struct file *filp, char __user *buf, 
                              size_t count, loff_t *f_pos)
{
    struct char_device_data *dev_data = filp->private_data;
    size_t bytes_to_read;
    ssize_t retval = 0;
    
    if (mutex_lock_interruptible(&dev_data->lock))
        return -ERESTARTSYS;
    
    /* 检查读取位置 */
    if (*f_pos >= dev_data->data_size)
        goto out;
    
    /* 计算实际读取字节数 */
    bytes_to_read = min(count, dev_data->data_size - (size_t)*f_pos);
    
    /* 复制数据到用户空间 */
    if (copy_to_user(buf, dev_data->buffer + *f_pos, bytes_to_read)) {
        retval = -EFAULT;
        goto out;
    }
    
    *f_pos += bytes_to_read;
    retval = bytes_to_read;
    
    pr_info("CharDev: Read %zu bytes at offset %lld\n", bytes_to_read, *f_pos - bytes_to_read);
    
out:
    mutex_unlock(&dev_data->lock);
    return retval;
}

/*
 * 写入设备
 */
static ssize_t char_dev_write(struct file *filp, const char __user *buf,
                               size_t count, loff_t *f_pos)
{
    struct char_device_data *dev_data = filp->private_data;
    size_t bytes_to_write;
    ssize_t retval = 0;
    
    if (mutex_lock_interruptible(&dev_data->lock))
        return -ERESTARTSYS;
    
    /* 检查写入位置 */
    if (*f_pos >= dev_data->buffer_size) {
        retval = -ENOSPC;
        goto out;
    }
    
    /* 计算实际写入字节数 */
    bytes_to_write = min(count, dev_data->buffer_size - (size_t)*f_pos);
    
    /* 从用户空间复制数据 */
    if (copy_from_user(dev_data->buffer + *f_pos, buf, bytes_to_write)) {
        retval = -EFAULT;
        goto out;
    }
    
    *f_pos += bytes_to_write;
    
    /* 更新数据大小 */
    if (*f_pos > dev_data->data_size)
        dev_data->data_size = *f_pos;
    
    retval = bytes_to_write;
    
    pr_info("CharDev: Wrote %zu bytes at offset %lld\n", bytes_to_write, *f_pos - bytes_to_write);
    
out:
    mutex_unlock(&dev_data->lock);
    return retval;
}

/*
 * 改变文件位置（lseek）
 */
static loff_t char_dev_llseek(struct file *filp, loff_t offset, int whence)
{
    struct char_device_data *dev_data = filp->private_data;
    loff_t newpos;
    
    switch (whence) {
    case SEEK_SET:
        newpos = offset;
        break;
    case SEEK_CUR:
        newpos = filp->f_pos + offset;
        break;
    case SEEK_END:
        newpos = dev_data->data_size + offset;
        break;
    default:
        return -EINVAL;
    }
    
    if (newpos < 0)
        return -EINVAL;
    if (newpos > dev_data->buffer_size)
        return -EINVAL;
    
    filp->f_pos = newpos;
    pr_info("CharDev: Seek to position %lld\n", newpos);
    
    return newpos;
}

/*
 * ioctl 控制命令
 */
static long char_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct char_device_data *dev_data = filp->private_data;
    int retval = 0;
    int tmp;
    
    /* 检查魔数 */
    if (_IOC_TYPE(cmd) != CHARDEV_IOC_MAGIC)
        return -ENOTTY;
    
    if (mutex_lock_interruptible(&dev_data->lock))
        return -ERESTARTSYS;
    
    switch (cmd) {
    case CHARDEV_IOCRESET:
        /* 清除缓冲区 */
        memset(dev_data->buffer, 0, dev_data->buffer_size);
        dev_data->data_size = 0;
        filp->f_pos = 0;
        pr_info("CharDev: Buffer reset\n");
        break;
        
    case CHARDEV_IOCGETSIZE:
        /* 获取缓冲区大小 */
        tmp = dev_data->buffer_size;
        if (copy_to_user((int __user *)arg, &tmp, sizeof(tmp))) {
            retval = -EFAULT;
        }
        break;
        
    case CHARDEV_IOCGETUSED:
        /* 获取已使用大小 */
        tmp = dev_data->data_size;
        if (copy_to_user((int __user *)arg, &tmp, sizeof(tmp))) {
            retval = -EFAULT;
        }
        break;
        
    default:
        retval = -ENOTTY;
        break;
    }
    
    mutex_unlock(&dev_data->lock);
    return retval;
}

/* 文件操作结构 */
static struct file_operations char_dev_fops = {
    .owner          = THIS_MODULE,
    .open           = char_dev_open,
    .release        = char_dev_release,
    .read           = char_dev_read,
    .write          = char_dev_write,
    .llseek         = char_dev_llseek,
    .unlocked_ioctl = char_dev_ioctl,
};

/*
 * 模块初始化
 */
static int __init char_driver_init(void)
{
    int ret;
    
    pr_info("CharDev: Initializing character device driver\n");
    
    /* 分配设备私有数据 */
    char_dev_data = kzalloc(sizeof(struct char_device_data), GFP_KERNEL);
    if (!char_dev_data) {
        pr_err("CharDev: Failed to allocate device data\n");
        return -ENOMEM;
    }
    
    /* 分配设备缓冲区 */
    char_dev_data->buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!char_dev_data->buffer) {
        pr_err("CharDev: Failed to allocate buffer\n");
        ret = -ENOMEM;
        goto fail_buffer;
    }
    
    char_dev_data->buffer_size = BUFFER_SIZE;
    char_dev_data->data_size = 0;
    mutex_init(&char_dev_data->lock);
    
    /* 动态分配设备号 */
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("CharDev: Failed to allocate device number\n");
        goto fail_chrdev;
    }
    
    pr_info("CharDev: Device number allocated: MAJOR=%d, MINOR=%d\n",
            MAJOR(dev_num), MINOR(dev_num));
    
    /* 初始化并添加字符设备 */
    cdev_init(&char_dev_data->cdev, &char_dev_fops);
    char_dev_data->cdev.owner = THIS_MODULE;
    
    ret = cdev_add(&char_dev_data->cdev, dev_num, 1);
    if (ret) {
        pr_err("CharDev: Failed to add character device\n");
        goto fail_cdev_add;
    }
    
    /* 创建设备类 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    char_class = class_create(CLASS_NAME);
#else
    char_class = class_create(THIS_MODULE, CLASS_NAME);
#endif
    if (IS_ERR(char_class)) {
        pr_err("CharDev: Failed to create device class\n");
        ret = PTR_ERR(char_class);
        goto fail_class;
    }
    
    /* 创建设备节点 */
    char_device = device_create(char_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(char_device)) {
        pr_err("CharDev: Failed to create device\n");
        ret = PTR_ERR(char_device);
        goto fail_device;
    }
    
    
    pr_info("CharDev: Device /dev/%s created successfully\n", DEVICE_NAME);
    pr_info("CharDev: Buffer size: %zu bytes\n", char_dev_data->buffer_size);
    
    return 0;
    
fail_device:
    class_destroy(char_class);
fail_class:
    cdev_del(&char_dev_data->cdev);
fail_cdev_add:
    unregister_chrdev_region(dev_num, 1);
fail_chrdev:
    kfree(char_dev_data->buffer);
fail_buffer:
    kfree(char_dev_data);
    return ret;
}

/*
 * 模块退出
 */
static void __exit char_driver_exit(void)
{
    pr_info("CharDev: Cleaning up character device driver\n");
    
    /* 删除设备节点 */
    device_destroy(char_class, dev_num);
    
    /* 删除设备类 */
    class_destroy(char_class);
    
    /* 删除字符设备 */
    cdev_del(&char_dev_data->cdev);
    
    /* 释放设备号 */
    unregister_chrdev_region(dev_num, 1);
    
    /* 释放缓冲区和私有数据 */
    kfree(char_dev_data->buffer);
    kfree(char_dev_data);
    
    pr_info("CharDev: Character device driver removed\n");
}

module_init(char_driver_init);
module_exit(char_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huaizhen.LV");
MODULE_DESCRIPTION("Virtual Character Device Driver with Memory Backend");
MODULE_VERSION("1.0");

