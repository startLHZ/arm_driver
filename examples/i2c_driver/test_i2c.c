#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <string.h>

#define I2C_BUS "/dev/i2c-0"
#define I2C_ADDR 0x50

int main(int argc, char *argv[])
{
    int fd;
    unsigned char reg = 0x00;
    unsigned char data = 0x00;
    
    // 打开I2C设备
    fd = open(I2C_BUS, O_RDWR);
    if (fd < 0) {
        perror("打开I2C设备失败");
        return -1;
    }
    
    // 设置I2C从设备地址
    if (ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0) {
        perror("设置I2C地址失败");
        close(fd);
        return -1;
    }
    
    // 写入数据示例
    unsigned char write_buf[2] = {0x00, 0xAA};  // 寄存器地址0x00，数据0xAA
    if (write(fd, write_buf, 2) != 2) {
        perror("写入I2C数据失败");
    } else {
        printf("成功写入: 寄存器=0x%02X, 数据=0x%02X\n", write_buf[0], write_buf[1]);
    }
    
    usleep(10000);  // 延时10ms
    
    // 读取数据示例
    unsigned char read_buf[1];
    reg = 0x00;
    if (write(fd, &reg, 1) != 1) {
        perror("写入寄存器地址失败");
    } else if (read(fd, read_buf, 1) != 1) {
        perror("读取I2C数据失败");
    } else {
        printf("成功读取: 寄存器=0x%02X, 数据=0x%02X\n", reg, read_buf[0]);
    }
    
    close(fd);
    return 0;
}

