#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <string.h>

#define I2C_BUS "/dev/i2c-2"
#define I2C_ADDR 0x29

int main(int argc, char *argv[])
{
    int fd;
    unsigned char reg[2] = {0x00, 0x0d};  // 16位寄存器地址：高字节0x00，低字节0x0d
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
    
    // 使用I2C消息读取寄存器数据
    unsigned char read_buf[1];
    
    struct i2c_msg msgs[2] = {
        {
            .addr = I2C_ADDR,
            .flags = 0,           // 写操作
            .len = 2,             // 写入2字节的寄存器地址
            .buf = reg,
        },
        {
            .addr = I2C_ADDR,
            .flags = I2C_M_RD,    // 读操作
            .len = 1,
            .buf = read_buf,
        }
    };
    
    struct i2c_rdwr_ioctl_data msgset = {
        .msgs = msgs,
        .nmsgs = 2,
    };
    
    if (ioctl(fd, I2C_RDWR, &msgset) < 0) {
        perror("I2C读写操作失败 error: ");
        close(fd);
        return -1;
    } else {
        printf("成功读取: 寄存器=0x%02X%02X, 数据=0x%02X\n", reg[0], reg[1], read_buf[0]);
    }
    
    close(fd);
    return 0;
}

