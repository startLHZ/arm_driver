/*
 * Block Device Driver Header File
 * 
 * This header defines constants and structures for the block device driver
 */

#ifndef BLOCK_DRIVER_H
#define BLOCK_DRIVER_H

/* Device name as it appears in /dev/ */
#define MYBLK_DEVICE_NAME "myblkdev"

/* Sector size (standard is 512 bytes) */
#define MYBLK_SECTOR_SIZE 512

/* Device size configuration */
#define MYBLK_NSECTORS (1024 * 1024)  /* Total sectors (512MB) */
#define MYBLK_DEVICE_SIZE (MYBLK_NSECTORS * MYBLK_SECTOR_SIZE)

/* Number of minor devices */
#define MYBLK_MINORS 1

/* IOCTL commands (for future extension) */
#define MYBLK_IOCTL_GET_SIZE    _IOR('M', 0, unsigned long)
#define MYBLK_IOCTL_CLEAR       _IO('M', 1)

#endif /* BLOCK_DRIVER_H */
