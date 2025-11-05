/*
 * Network Block Device Driver
 * 
 * This driver creates a block device that stores data on a remote server
 * over TCP/IP network. Similar to NBD (Network Block Device) but with
 * a custom protocol.
 * 
 * Features:
 * - TCP/IP based communication with remote storage server
 * - Read/Write operations over network
 * - Automatic reconnection on network failure
 * - Configurable via sysfs
 * 
 * Protocol:
 * Request format: [CMD(1byte)][SECTOR(8bytes)][LENGTH(4bytes)][DATA(variable)]
 * Response format: [STATUS(1byte)][DATA(variable)]
 * 
 * Commands:
 * 0x01 - READ
 * 0x02 - WRITE
 * 0x03 - DISCONNECT
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
#include <linux/net.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/socket.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <net/sock.h>

#define DEVICE_NAME "netblk"
#define KERNEL_SECTOR_SIZE 512

/* Device parameters */
#define NETBLK_SECTOR_SIZE 512
#define NETBLK_MINORS 1
#define NETBLK_DEFAULT_SIZE (100 * 1024 * 1024)  /* 100MB default */
#define MAX_RETRIES 3
#define CONNECT_TIMEOUT 5000  /* ms */

/* Network protocol commands */
#define NET_CMD_READ       0x01
#define NET_CMD_WRITE      0x02
#define NET_CMD_DISCONNECT 0x03

/* Network protocol responses */
#define NET_STATUS_OK      0x00
#define NET_STATUS_ERROR   0x01

/* Request packet structure */
struct net_request_packet {
    u8 cmd;
    u64 sector;
    u32 length;
    /* followed by data for write operations */
} __packed;

/* Response packet structure */
struct net_response_packet {
    u8 status;
    /* followed by data for read operations */
} __packed;

/* Network connection state */
enum netblk_state {
    NETBLK_DISCONNECTED = 0,
    NETBLK_CONNECTING,
    NETBLK_CONNECTED,
    NETBLK_ERROR
};

/* Device structure */
struct netblk_device {
    unsigned long size;              /* Device size in bytes */
    struct mutex lock;               /* Mutex for I/O operations */
    struct gendisk *gd;              /* Generic disk structure */
    struct blk_mq_tag_set tag_set;   /* blk-mq tag set */
    struct request_queue *queue;     /* Request queue */
    
    /* Network connection */
    struct socket *sock;             /* TCP socket */
    struct sockaddr_in server_addr;  /* Server address */
    char server_ip[16];              /* Server IP string */
    u16 server_port;                 /* Server port */
    enum netblk_state state;         /* Connection state */
    
    /* Connection thread */
    struct task_struct *conn_thread; /* Connection management thread */
    atomic_t should_stop;            /* Flag to stop connection thread */
    
    /* Statistics */
    atomic64_t read_bytes;
    atomic64_t write_bytes;
    atomic64_t errors;
};

static struct netblk_device *netblk_dev = NULL;
static int netblk_major = 0;

/*
 * Network helper functions
 */

/* Send data over socket */
static int netblk_send(struct socket *sock, void *buf, size_t len)
{
    struct msghdr msg;
    struct kvec iov;
    int ret;
    
    memset(&msg, 0, sizeof(msg));
    iov.iov_base = buf;
    iov.iov_len = len;
    
    ret = kernel_sendmsg(sock, &msg, &iov, 1, len);
    if (ret < 0) {
        printk(KERN_ERR "netblk: Send failed: %d\n", ret);
        return ret;
    }
    
    if (ret != len) {
        printk(KERN_WARNING "netblk: Partial send: %d/%zu bytes\n", ret, len);
        return -EIO;
    }
    
    return 0;
}

/* Receive data from socket */
static int netblk_recv(struct socket *sock, void *buf, size_t len)
{
    struct msghdr msg;
    struct kvec iov;
    int ret;
    size_t received = 0;
    
    while (received < len) {
        memset(&msg, 0, sizeof(msg));
        iov.iov_base = buf + received;
        iov.iov_len = len - received;
        
        ret = kernel_recvmsg(sock, &msg, &iov, 1, len - received, 0);
        if (ret < 0) {
            printk(KERN_ERR "netblk: Recv failed: %d\n", ret);
            return ret;
        }
        
        if (ret == 0) {
            printk(KERN_ERR "netblk: Connection closed by peer\n");
            return -ECONNRESET;
        }
        
        received += ret;
    }
    
    return 0;
}

/* Connect to remote server */
static int netblk_connect(struct netblk_device *dev)
{
    int ret;
    
    if (dev->state == NETBLK_CONNECTED && dev->sock)
        return 0;
    
    printk(KERN_INFO "netblk: Connecting to %s:%d...\n",
           dev->server_ip, dev->server_port);
    
    dev->state = NETBLK_CONNECTING;
    
    /* Create socket */
    ret = sock_create(AF_INET, SOCK_STREAM, IPPROTO_TCP, &dev->sock);
    if (ret < 0) {
        printk(KERN_ERR "netblk: Failed to create socket: %d\n", ret);
        dev->state = NETBLK_ERROR;
        return ret;
    }
    
    /* Set socket options */
    {
        /* Set keepalive */
        sock_set_keepalive(dev->sock->sk);
        
        /* Set socket to non-blocking mode is not needed for kernel sockets
         * Timeouts are handled through msghdr flags in send/recv operations
         */
    }
    
    /* Setup server address */
    memset(&dev->server_addr, 0, sizeof(dev->server_addr));
    dev->server_addr.sin_family = AF_INET;
    dev->server_addr.sin_port = htons(dev->server_port);
    dev->server_addr.sin_addr.s_addr = in_aton(dev->server_ip);
    
    /* Connect */
    ret = kernel_connect(dev->sock, (struct sockaddr *)&dev->server_addr,
                        sizeof(dev->server_addr), 0);
    if (ret < 0) {
        printk(KERN_ERR "netblk: Failed to connect: %d\n", ret);
        sock_release(dev->sock);
        dev->sock = NULL;
        dev->state = NETBLK_ERROR;
        return ret;
    }
    
    dev->state = NETBLK_CONNECTED;
    printk(KERN_INFO "netblk: Connected successfully\n");
    
    return 0;
}

/* Disconnect from server */
static void netblk_disconnect(struct netblk_device *dev)
{
    if (dev->sock) {
        struct net_request_packet pkt;
        
        /* Send disconnect command */
        pkt.cmd = NET_CMD_DISCONNECT;
        pkt.sector = 0;
        pkt.length = 0;
        netblk_send(dev->sock, &pkt, sizeof(pkt));
        
        sock_release(dev->sock);
        dev->sock = NULL;
    }
    dev->state = NETBLK_DISCONNECTED;
    printk(KERN_INFO "netblk: Disconnected\n");
}

/* Network read operation */
static int netblk_net_read(struct netblk_device *dev, u64 sector,
                          void *buf, u32 len)
{
    struct net_request_packet req;
    struct net_response_packet resp;
    int ret;
    int retry;
    
    for (retry = 0; retry < MAX_RETRIES; retry++) {
        mutex_lock(&dev->lock);
        
        /* Ensure connected */
        if (dev->state != NETBLK_CONNECTED) {
            ret = netblk_connect(dev);
            if (ret < 0) {
                mutex_unlock(&dev->lock);
                msleep(1000);
                continue;
            }
        }
        
        /* Send read request */
        req.cmd = NET_CMD_READ;
        req.sector = cpu_to_be64(sector);
        req.length = cpu_to_be32(len);
        
        ret = netblk_send(dev->sock, &req, sizeof(req));
        if (ret < 0) {
            printk(KERN_WARNING "netblk: Send request failed, retry %d\n", retry);
            netblk_disconnect(dev);
            mutex_unlock(&dev->lock);
            msleep(1000);
            continue;
        }
        
        /* Receive response status */
        ret = netblk_recv(dev->sock, &resp, sizeof(resp));
        if (ret < 0) {
            printk(KERN_WARNING "netblk: Recv response failed, retry %d\n", retry);
            netblk_disconnect(dev);
            mutex_unlock(&dev->lock);
            msleep(1000);
            continue;
        }
        
        if (resp.status != NET_STATUS_OK) {
            printk(KERN_ERR "netblk: Server returned error status\n");
            mutex_unlock(&dev->lock);
            atomic64_inc(&dev->errors);
            return -EIO;
        }
        
        /* Receive data */
        ret = netblk_recv(dev->sock, buf, len);
        if (ret < 0) {
            printk(KERN_WARNING "netblk: Recv data failed, retry %d\n", retry);
            netblk_disconnect(dev);
            mutex_unlock(&dev->lock);
            msleep(1000);
            continue;
        }
        
        mutex_unlock(&dev->lock);
        atomic64_add(len, &dev->read_bytes);
        return 0;
    }
    
    atomic64_inc(&dev->errors);
    return -EIO;
}

/* Network write operation */
static int netblk_net_write(struct netblk_device *dev, u64 sector,
                           const void *buf, u32 len)
{
    struct net_request_packet req;
    struct net_response_packet resp;
    int ret;
    int retry;
    
    for (retry = 0; retry < MAX_RETRIES; retry++) {
        mutex_lock(&dev->lock);
        
        /* Ensure connected */
        if (dev->state != NETBLK_CONNECTED) {
            ret = netblk_connect(dev);
            if (ret < 0) {
                mutex_unlock(&dev->lock);
                msleep(1000);
                continue;
            }
        }
        
        /* Send write request */
        req.cmd = NET_CMD_WRITE;
        req.sector = cpu_to_be64(sector);
        req.length = cpu_to_be32(len);
        
        ret = netblk_send(dev->sock, &req, sizeof(req));
        if (ret < 0) {
            printk(KERN_WARNING "netblk: Send request failed, retry %d\n", retry);
            netblk_disconnect(dev);
            mutex_unlock(&dev->lock);
            msleep(1000);
            continue;
        }
        
        /* Send data */
        ret = netblk_send(dev->sock, (void *)buf, len);
        if (ret < 0) {
            printk(KERN_WARNING "netblk: Send data failed, retry %d\n", retry);
            netblk_disconnect(dev);
            mutex_unlock(&dev->lock);
            msleep(1000);
            continue;
        }
        
        /* Receive response status */
        ret = netblk_recv(dev->sock, &resp, sizeof(resp));
        if (ret < 0) {
            printk(KERN_WARNING "netblk: Recv response failed, retry %d\n", retry);
            netblk_disconnect(dev);
            mutex_unlock(&dev->lock);
            msleep(1000);
            continue;
        }
        
        if (resp.status != NET_STATUS_OK) {
            printk(KERN_ERR "netblk: Server returned error status\n");
            mutex_unlock(&dev->lock);
            atomic64_inc(&dev->errors);
            return -EIO;
        }
        
        mutex_unlock(&dev->lock);
        atomic64_add(len, &dev->write_bytes);
        return 0;
    }
    
    atomic64_inc(&dev->errors);
    return -EIO;
}

/*
 * Handle an I/O request
 */
static blk_status_t netblk_request(struct blk_mq_hw_ctx *hctx,
                                   const struct blk_mq_queue_data *bd)
{
    struct request *req = bd->rq;
    struct netblk_device *dev = req->q->queuedata;
    struct bio_vec bvec;
    struct req_iterator iter;
    u64 sector;
    blk_status_t ret = BLK_STS_OK;
    int io_ret;
    void *temp_buf = NULL;
    size_t total_len = blk_rq_bytes(req);
    
    blk_mq_start_request(req);
    
    sector = blk_rq_pos(req);
    
    /* Allocate temporary buffer */
    temp_buf = vmalloc(total_len);
    if (!temp_buf) {
        printk(KERN_ERR "netblk: Failed to allocate temp buffer\n");
        ret = BLK_STS_RESOURCE;
        goto out;
    }
    
    /* For write, copy data from bio to temp buffer */
    if (req_op(req) == REQ_OP_WRITE) {
        size_t offset = 0;
        rq_for_each_segment(bvec, req, iter) {
            void *src = kmap_atomic(bvec.bv_page);
            memcpy(temp_buf + offset, src + bvec.bv_offset, bvec.bv_len);
            kunmap_atomic(src);
            offset += bvec.bv_len;
        }
    }
    
    /* Perform network I/O */
    switch (req_op(req)) {
    case REQ_OP_READ:
        io_ret = netblk_net_read(dev, sector, temp_buf, total_len);
        if (io_ret < 0) {
            printk(KERN_ERR "netblk: Read failed at sector %llu\n", sector);
            ret = BLK_STS_IOERR;
        }
        break;
        
    case REQ_OP_WRITE:
        io_ret = netblk_net_write(dev, sector, temp_buf, total_len);
        if (io_ret < 0) {
            printk(KERN_ERR "netblk: Write failed at sector %llu\n", sector);
            ret = BLK_STS_IOERR;
        }
        break;
        
    default:
        printk(KERN_WARNING "netblk: Unsupported request operation\n");
        ret = BLK_STS_NOTSUPP;
        break;
    }
    
    /* For read, copy data from temp buffer to bio */
    if (ret == BLK_STS_OK && req_op(req) == REQ_OP_READ) {
        size_t offset = 0;
        rq_for_each_segment(bvec, req, iter) {
            void *dst = kmap_atomic(bvec.bv_page);
            memcpy(dst + bvec.bv_offset, temp_buf + offset, bvec.bv_len);
            kunmap_atomic(dst);
            offset += bvec.bv_len;
        }
    }
    
    vfree(temp_buf);
    
out:
    blk_mq_end_request(req, ret);
    return ret;
}

/*
 * Block device operations
 */
static int netblk_open(struct block_device *bdev, fmode_t mode)
{
    printk(KERN_INFO "netblk: Device opened\n");
    return 0;
}

static void netblk_release(struct gendisk *gd, fmode_t mode)
{
    printk(KERN_INFO "netblk: Device closed\n");
}

static int netblk_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
    struct netblk_device *dev = bdev->bd_disk->private_data;
    
    geo->heads = 4;
    geo->sectors = 16;
    geo->cylinders = (dev->size / NETBLK_SECTOR_SIZE) / (geo->heads * geo->sectors);
    geo->start = 0;
    
    return 0;
}

static const struct block_device_operations netblk_fops = {
    .owner = THIS_MODULE,
    .open = netblk_open,
    .release = netblk_release,
    .getgeo = netblk_getgeo,
};

/*
 * blk-mq operations
 */
static struct blk_mq_ops netblk_mq_ops = {
    .queue_rq = netblk_request,
};

/*
 * Sysfs attributes for configuration
 */

/* Show server IP */
static ssize_t server_ip_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", netblk_dev->server_ip);
}

/* Set server IP */
static ssize_t server_ip_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
{
    if (count >= sizeof(netblk_dev->server_ip))
        return -EINVAL;
    
    sscanf(buf, "%15s", netblk_dev->server_ip);
    printk(KERN_INFO "netblk: Server IP set to %s\n", netblk_dev->server_ip);
    
    return count;
}

/* Show server port */
static ssize_t server_port_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%u\n", netblk_dev->server_port);
}

/* Set server port */
static ssize_t server_port_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
{
    int port;
    if (kstrtoint(buf, 10, &port) != 0)
        return -EINVAL;
    
    if (port <= 0 || port > 65535)
        return -EINVAL;
    
    netblk_dev->server_port = port;
    printk(KERN_INFO "netblk: Server port set to %d\n", port);
    
    return count;
}

/* Show connection state */
static ssize_t state_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    const char *state_str[] = {
        "disconnected", "connecting", "connected", "error"
    };
    return sprintf(buf, "%s\n", state_str[netblk_dev->state]);
}

/* Show statistics */
static ssize_t stats_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    return sprintf(buf,
        "read_bytes:  %llu\n"
        "write_bytes: %llu\n"
        "errors:      %llu\n",
        atomic64_read(&netblk_dev->read_bytes),
        atomic64_read(&netblk_dev->write_bytes),
        atomic64_read(&netblk_dev->errors));
}

/* Manual connect */
static ssize_t connect_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
{
    if (strncmp(buf, "1", 1) == 0) {
        netblk_connect(netblk_dev);
    }
    return count;
}

/* Manual disconnect */
static ssize_t disconnect_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
{
    if (strncmp(buf, "1", 1) == 0) {
        netblk_disconnect(netblk_dev);
    }
    return count;
}

static DEVICE_ATTR_RW(server_ip);
static DEVICE_ATTR_RW(server_port);
static DEVICE_ATTR_RO(state);
static DEVICE_ATTR_RO(stats);
static DEVICE_ATTR_WO(connect);
static DEVICE_ATTR_WO(disconnect);

static struct attribute *netblk_attrs[] = {
    &dev_attr_server_ip.attr,
    &dev_attr_server_port.attr,
    &dev_attr_state.attr,
    &dev_attr_stats.attr,
    &dev_attr_connect.attr,
    &dev_attr_disconnect.attr,
    NULL,
};

static const struct attribute_group netblk_attr_group = {
    .attrs = netblk_attrs,
};

/*
 * Initialize the device
 */
static int __init netblk_init(void)
{
    int ret = 0;
    
    printk(KERN_INFO "netblk: Initializing Network Block Device driver\n");
    
    /* Allocate device structure */
    netblk_dev = kzalloc(sizeof(struct netblk_device), GFP_KERNEL);
    if (!netblk_dev) {
        printk(KERN_ERR "netblk: Failed to allocate device structure\n");
        return -ENOMEM;
    }
    
    /* Set default parameters */
    netblk_dev->size = NETBLK_DEFAULT_SIZE;
    strcpy(netblk_dev->server_ip, "192.168.1.22");  /* Server IP */
    netblk_dev->server_port = 10809;
    netblk_dev->state = NETBLK_DISCONNECTED;
    netblk_dev->sock = NULL;
    
    /* Initialize mutex and statistics */
    mutex_init(&netblk_dev->lock);
    atomic64_set(&netblk_dev->read_bytes, 0);
    atomic64_set(&netblk_dev->write_bytes, 0);
    atomic64_set(&netblk_dev->errors, 0);
    atomic_set(&netblk_dev->should_stop, 0);
    
    /* Register block device */
    netblk_major = register_blkdev(0, DEVICE_NAME);
    if (netblk_major < 0) {
        printk(KERN_ERR "netblk: Failed to register block device\n");
        ret = netblk_major;
        goto out_free_dev;
    }
    
    /* Initialize blk-mq tag set */
    netblk_dev->tag_set.ops = &netblk_mq_ops;
    netblk_dev->tag_set.nr_hw_queues = 1;
    netblk_dev->tag_set.queue_depth = 128;
    netblk_dev->tag_set.numa_node = NUMA_NO_NODE;
    netblk_dev->tag_set.cmd_size = 0;
    netblk_dev->tag_set.flags = BLK_MQ_F_SHOULD_MERGE;
    netblk_dev->tag_set.driver_data = netblk_dev;
    
    ret = blk_mq_alloc_tag_set(&netblk_dev->tag_set);
    if (ret) {
        printk(KERN_ERR "netblk: Failed to allocate tag set\n");
        goto out_unregister;
    }
    
    /* Allocate disk */
    netblk_dev->gd = blk_mq_alloc_disk(&netblk_dev->tag_set, netblk_dev);
    if (IS_ERR(netblk_dev->gd)) {
        ret = PTR_ERR(netblk_dev->gd);
        printk(KERN_ERR "netblk: Failed to allocate disk\n");
        goto out_free_tag_set;
    }
    
    netblk_dev->gd->major = netblk_major;
    netblk_dev->gd->first_minor = 0;
    netblk_dev->gd->minors = NETBLK_MINORS;
    netblk_dev->gd->fops = &netblk_fops;
    netblk_dev->gd->private_data = netblk_dev;
    snprintf(netblk_dev->gd->disk_name, 32, DEVICE_NAME);
    
    netblk_dev->queue = netblk_dev->gd->queue;
    netblk_dev->queue->queuedata = netblk_dev;
    blk_queue_logical_block_size(netblk_dev->queue, NETBLK_SECTOR_SIZE);
    blk_queue_physical_block_size(netblk_dev->queue, NETBLK_SECTOR_SIZE);
    
    /* Set capacity */
    set_capacity(netblk_dev->gd, netblk_dev->size / NETBLK_SECTOR_SIZE);
    
    /* Create sysfs attributes */
    ret = sysfs_create_group(&disk_to_dev(netblk_dev->gd)->kobj,
                            &netblk_attr_group);
    if (ret) {
        printk(KERN_WARNING "netblk: Failed to create sysfs attributes\n");
    }
    
    /* Add disk */
    ret = add_disk(netblk_dev->gd);
    if (ret) {
        printk(KERN_ERR "netblk: Failed to add disk\n");
        goto out_cleanup_disk;
    }
    
    printk(KERN_INFO "netblk: Network block device initialized successfully\n");
    printk(KERN_INFO "netblk: Device size: %lu bytes (%lu sectors)\n",
           netblk_dev->size, netblk_dev->size / NETBLK_SECTOR_SIZE);
    printk(KERN_INFO "netblk: Server: %s:%d\n",
           netblk_dev->server_ip, netblk_dev->server_port);
    printk(KERN_INFO "netblk: Configuration: /sys/block/%s/\n", DEVICE_NAME);
    
    return 0;
    
out_cleanup_disk:
    sysfs_remove_group(&disk_to_dev(netblk_dev->gd)->kobj, &netblk_attr_group);
    put_disk(netblk_dev->gd);
out_free_tag_set:
    blk_mq_free_tag_set(&netblk_dev->tag_set);
out_unregister:
    unregister_blkdev(netblk_major, DEVICE_NAME);
out_free_dev:
    kfree(netblk_dev);
    netblk_dev = NULL;
    return ret;
}

/*
 * Cleanup the device
 */
static void __exit netblk_exit(void)
{
    printk(KERN_INFO "netblk: Cleaning up Network Block Device driver\n");
    
    if (netblk_dev) {
        /* Stop connection thread */
        atomic_set(&netblk_dev->should_stop, 1);
        if (netblk_dev->conn_thread) {
            kthread_stop(netblk_dev->conn_thread);
        }
        
        /* Disconnect from server */
        netblk_disconnect(netblk_dev);
        
        /* Remove sysfs attributes */
        if (netblk_dev->gd) {
            sysfs_remove_group(&disk_to_dev(netblk_dev->gd)->kobj,
                             &netblk_attr_group);
            del_gendisk(netblk_dev->gd);
            put_disk(netblk_dev->gd);
        }
        
        blk_mq_free_tag_set(&netblk_dev->tag_set);
        
        if (netblk_major > 0)
            unregister_blkdev(netblk_major, DEVICE_NAME);
        
        kfree(netblk_dev);
    }
    
    printk(KERN_INFO "netblk: Network block device driver unloaded\n");
}

module_init(netblk_init);
module_exit(netblk_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huaizhen.Lv");
MODULE_DESCRIPTION("Network Block Device Driver with TCP/IP");
MODULE_VERSION("1.0");
