# Flash 数据读取指南

本指南介绍如何从 flash 地址 0x0C0003 或其他任意地址读取数据。

## 方法1: 使用命令行（最简单）

驱动模块提供了 sysfs 接口，可以直接通过 shell 命令读取：

```bash
# 1. 加载驱动模块
sudo insmod block_driver.ko

# 2. 读取 0x0C0003 地址的 32 字节数据
echo 0x0C0003 32 > /sys/class/flashblk/flashblk/flash_data
cat /sys/class/flashblk/flashblk/flash_data

# 3. 读取其他地址（例如 0x000000 的 256 字节）
echo 0x000000 256 > /sys/class/flashblk/flashblk/flash_data
cat /sys/class/flashblk/flashblk/flash_data
```

## 方法2: 使用 read_flash 程序

已提供了一个专门的 C 程序 `read_flash` 来读取 flash 数据。

### 编译程序

```bash
# 在项目根目录执行
./build.sh
```

编译后的程序位于：`build/output/read_flash`

### 使用方法

```bash
# 基本语法
./read_flash <地址> <长度>

# 示例1: 读取 0x0C0003 的 16 字节
./read_flash 0x0C0003 16

# 示例2: 读取 0x0C0003 的 32 字节
./read_flash 0x0C0003 32

# 示例3: 读取 0x000000 的 256 字节
./read_flash 0x000000 256

# 示例4: 使用十进制地址（786435 = 0x0C0003）
./read_flash 786435 16
```

### 输出格式

程序会以十六进制格式显示读取的数据：

```
Reading 16 bytes from flash address 0x0C0003...

--- Flash Data ---
Flash Read: addr=0x000C0003 len=16

0000: 48 65 6C 6C 6F 20 57 6F 72 6C 64 21 0A 00 00 00 
------------------
```

## 方法3: 分步设置参数

如果需要更精细的控制，可以分别设置地址和长度：

```bash
# 设置要读取的地址
echo 0x0C0003 > /sys/class/flashblk/flashblk/flash_addr

# 设置要读取的长度
echo 32 > /sys/class/flashblk/flashblk/flash_len

# 读取数据
cat /sys/class/flashblk/flashblk/flash_data
```

## 参数说明

- **地址格式**：支持十六进制（0xXXXXXX）或十进制
- **长度限制**：1-256 字节
- **Flash 地址范围**：根据驱动配置，默认支持 0x000000 - 0xC001C (786813 字节)

## 注意事项

1. 驱动模块必须先加载：`sudo insmod block_driver.ko`
2. 可能需要 root 权限访问 sysfs 文件
3. I2C 总线和地址必须正确配置（当前配置：I2C bus 4, addr 0x11）
4. 读取大量数据时可能需要等待 I2C 传输完成

## 故障排查

### 问题1: sysfs 文件不存在
```bash
# 检查驱动是否加载
lsmod | grep block_driver

# 检查 sysfs 目录
ls -la /sys/class/flashblk/flashblk/
```

### 问题2: 权限不足
```bash
# 使用 sudo 运行
sudo ./read_flash 0x0C0003 16

# 或修改权限（不推荐）
sudo chmod 666 /sys/class/flashblk/flashblk/flash_data
```

### 问题3: 读取失败
检查内核日志：
```bash
dmesg | tail -20
```

常见错误：
- I2C 通信失败：检查硬件连接和 I2C 地址
- 超出地址范围：确认 flash 容量
- 设备繁忙：等待后重试

## API 说明（如果要在自己的程序中集成）

```c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int read_flash_via_sysfs(unsigned int addr, unsigned int len) {
    char cmd[128];
    FILE *fp;
    
    // 写入地址和长度
    snprintf(cmd, sizeof(cmd), "0x%X %u", addr, len);
    fp = fopen("/sys/class/flashblk/flashblk/flash_data", "w");
    if (!fp) return -1;
    fprintf(fp, "%s", cmd);
    fclose(fp);
    
    // 读取结果
    usleep(100000); // 等待 100ms
    fp = fopen("/sys/class/flashblk/flashblk/flash_data", "r");
    if (!fp) return -1;
    
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), fp)) {
        printf("%s", buffer);
    }
    fclose(fp);
    
    return 0;
}

int main() {
    // 读取 0x0C0003 的 32 字节
    read_flash_via_sysfs(0x0C0003, 32);
    return 0;
}
```

## 相关文件

- **驱动源码**: `src/block_driver/block_driver.c`
- **读取程序**: `examples/block_driver/read_flash.c`
- **测试程序**: `examples/block_driver/test_block.c`
- **接口文档**: `src/block_driver/SYSFS_INTERFACE.md`
