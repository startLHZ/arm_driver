# 字符设备驱动 - 快速使用指南

## 📌 概述

本项目新增了一个**完整的字符设备驱动示例** (`char_driver`)，使用内存作为存储后端，演示了 Linux 字符设备驱动开发的核心技术。

### 🎯 特性亮点

- ✅ 基于内存的虚拟字符设备（4KB 缓冲区）
- ✅ 支持完整的文件操作（open/read/write/llseek/ioctl）
- ✅ 自动创建设备节点 `/dev/mychardev`
- ✅ 多进程并发安全（互斥锁保护）
- ✅ 完整的测试程序和自动化脚本
- ✅ 支持 ARM64 架构

## 🚀 快速开始

### 1. 编译驱动

```bash
# 在项目根目录执行
./build.sh
```

编译成功后，会在 `build/output/` 目录生成以下文件：
- `char_driver.ko` - 字符设备驱动模块
- `test_chardev` - 测试程序
- `test_char_device.sh` - 自动化测试脚本

### 2. 部署到 ARM64 设备

```bash
# 将文件传输到 ARM64 设备
scp build/output/char_driver.ko root@arm-device:/tmp/
scp build/output/test_chardev root@arm-device:/tmp/
scp build/output/test_char_device.sh root@arm-device:/tmp/

# 在 ARM64 设备上执行
ssh root@arm-device
cd /tmp
chmod +x test_char_device.sh test_chardev
```

### 3. 使用自动化脚本（推荐）

```bash
# 在 ARM64 设备上执行完整测试流程
sudo ./test_char_device.sh all

# 或者分步执行
sudo ./test_char_device.sh load      # 加载驱动
./test_char_device.sh info           # 查看设备信息
./test_chardev                       # 运行测试
sudo ./test_char_device.sh unload    # 卸载驱动
```

### 4. 手动测试流程

```bash
# 步骤 1: 加载驱动
sudo insmod char_driver.ko

# 步骤 2: 验证设备节点
ls -l /dev/mychardev
# 输出: crw------- 1 root root 245, 0 Nov 10 17:00 /dev/mychardev

# 步骤 3: 修改权限（允许普通用户访问）
sudo chmod 666 /dev/mychardev

# 步骤 4: 查看内核日志
dmesg | tail -10

# 步骤 5: 运行测试程序
./test_chardev

# 步骤 6: 卸载驱动
sudo rmmod char_driver
```

## 📖 详细使用

### 测试程序命令

```bash
# 运行所有测试（默认）
./test_chardev

# 查看设备信息
./test_chardev -i

# 写入自定义数据
./test_chardev -w "Hello World"

# 读取设备内容
./test_chardev -r

# 清空设备
./test_chardev -c

# 显示帮助
./test_chardev -h
```

### Shell 操作示例

```bash
# 写入数据
echo "Hello from shell" > /dev/mychardev

# 读取数据
cat /dev/mychardev

# 使用 dd 命令
dd if=/dev/zero of=/dev/mychardev bs=1024 count=4
dd if=/dev/mychardev of=output.txt
```

### C 语言编程示例

```c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    // 打开设备
    int fd = open("/dev/mychardev", O_RDWR);
    if (fd < 0) {
        perror("打开设备失败");
        return 1;
    }
    
    // 写入数据
    const char *msg = "Hello, Character Device!";
    ssize_t written = write(fd, msg, strlen(msg));
    printf("写入了 %zd 字节\n", written);
    
    // 重置到开头
    lseek(fd, 0, SEEK_SET);
    
    // 读取数据
    char buffer[100] = {0};
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer));
    printf("读取了 %zd 字节: %s\n", bytes_read, buffer);
    
    // 关闭设备
    close(fd);
    return 0;
}
```

### IOCTL 控制示例

```c
#include <sys/ioctl.h>

#define CHARDEV_IOC_MAGIC  'k'
#define CHARDEV_IOCRESET    _IO(CHARDEV_IOC_MAGIC,  0)
#define CHARDEV_IOCGETSIZE  _IOR(CHARDEV_IOC_MAGIC, 1, int)
#define CHARDEV_IOCGETUSED  _IOR(CHARDEV_IOC_MAGIC, 2, int)

int fd = open("/dev/mychardev", O_RDWR);

// 获取缓冲区大小
int size;
ioctl(fd, CHARDEV_IOCGETSIZE, &size);
printf("缓冲区大小: %d 字节\n", size);

// 获取已使用大小
int used;
ioctl(fd, CHARDEV_IOCGETUSED, &used);
printf("已使用: %d 字节\n", used);

// 清空缓冲区
ioctl(fd, CHARDEV_IOCRESET);

close(fd);
```

## 🧪 测试内容

测试程序包含以下 8 个测试：

1. **打开/关闭测试** - 验证设备基本访问
2. **写入测试** - 写入数据并验证返回值
3. **读取测试** - 读取数据并验证内容一致性
4. **Lseek 测试** - 测试 SEEK_SET/CUR/END 定位
5. **IOCTL 测试** - 测试所有 ioctl 命令
6. **边界测试** - 空设备读取、缓冲区溢出处理
7. **压力测试** - 100 次连续读写操作
8. **并发测试** - 多进程同时访问设备

### 测试输出示例

```
========================================
   字符设备驱动测试程序 v1.0
========================================

=== 测试 1: 打开/关闭设备 ===
✓ PASS: 打开设备
✓ PASS: 关闭设备

=== 测试 2: 写入数据 ===
✓ PASS: 写入数据
  写入了 32 字节: 'Hello, Character Device Driver!'

=== 测试 3: 读取数据 ===
✓ PASS: 读取数据并验证
  读取了 19 字节: 'Test Read Operation'

=== 测试 4: lseek 定位 ===
✓ PASS: SEEK_SET 定位到位置 5
✓ PASS: 从位置 5 读取正确数据
  读取到: '56789'
✓ PASS: SEEK_CUR 相对定位
✓ PASS: SEEK_END 定位到末尾
  文件末尾位置: 16

=== 测试 5: IOCTL 命令 ===
✓ PASS: 获取缓冲区大小
  缓冲区大小: 4096 字节
✓ PASS: 获取已使用大小
  已使用大小: 15 字节
✓ PASS: 重置缓冲区
✓ PASS: 重置后验证
  重置后大小: 0 字节

=== 测试 6: 边界条件 ===
✓ PASS: 读取空设备返回 0
✓ PASS: 填满缓冲区
  写入了 4096/4096 字节
✓ PASS: 缓冲区满后写入返回 0

=== 测试 7: 压力测试 ===
✓ PASS: 多次读写测试
  完成 100 次读写操作

=== 测试 8: 并发访问 ===
✓ PASS: 多进程并发访问

========================================
   测试结果统计
========================================
总测试数: 20
通过: 20
失败: 0
成功率: 100.0%

🎉 所有测试通过！
```

## 📊 内核日志示例

```bash
# 加载驱动时
$ sudo insmod char_driver.ko
$ dmesg | tail -5
[  123.456789] CharDev: Initializing character device driver
[  123.456790] CharDev: Device number allocated: MAJOR=245, MINOR=0
[  123.456791] CharDev: Device /dev/mychardev created successfully
[  123.456792] CharDev: Buffer size: 4096 bytes

# 使用设备时
$ echo "test" > /dev/mychardev
$ dmesg | tail -3
[  124.123456] CharDev: Device opened by process 1234 (bash)
[  124.123457] CharDev: Wrote 5 bytes at offset 0
[  124.123458] CharDev: Device closed by process 1234 (bash)

# 卸载驱动时
$ sudo rmmod char_driver
$ dmesg | tail -2
[  125.789012] CharDev: Cleaning up character device driver
[  125.789013] CharDev: Character device driver removed
```

## 🔍 故障排查

### 问题 1: 设备节点未创建

**现象**: `/dev/mychardev` 不存在

**解决**:
```bash
# 检查驱动是否加载
lsmod | grep char_driver

# 查看内核错误日志
dmesg | grep -i error | tail

# 手动创建设备节点（临时方案）
MAJOR=$(grep mychardev /proc/devices | awk '{print $1}')
sudo mknod /dev/mychardev c $MAJOR 0
sudo chmod 666 /dev/mychardev
```

### 问题 2: 权限被拒绝

**现象**: `Permission denied` 错误

**解决**:
```bash
# 方法1: 使用 sudo
sudo ./test_chardev

# 方法2: 修改设备权限
sudo chmod 666 /dev/mychardev

# 方法3: 创建 udev 规则（永久生效）
echo 'KERNEL=="mychardev", MODE="0666"' | sudo tee /etc/udev/rules.d/99-mychardev.rules
sudo udevadm control --reload-rules
```

### 问题 3: 设备繁忙

**现象**: `rmmod: ERROR: Module char_driver is in use`

**解决**:
```bash
# 查看哪个进程在使用
lsof /dev/mychardev

# 关闭使用设备的进程
kill -9 <PID>

# 强制卸载
sudo rmmod -f char_driver
```

### 问题 4: 加载失败

**现象**: `insmod: ERROR: could not insert module`

**解决**:
```bash
# 检查内核版本匹配
modinfo char_driver.ko

# 查看详细错误信息
dmesg | tail -20

# 检查符号依赖
modprobe --show-depends char_driver || echo "Module not installed"
```

## 📁 文件结构

```
linux_arm_driver/
├── src/char_driver/              # 驱动源码
│   ├── char_driver.c            # 字符设备驱动实现
│   ├── Makefile                 # 驱动编译配置
│   ├── CMakeLists.txt           # CMake 配置
│   └── README.md                # 详细文档
│
├── examples/char_driver/         # 测试程序
│   ├── test_chardev.c           # 测试程序源码
│   ├── test_char_device.sh      # 自动化测试脚本
│   └── CMakeLists.txt           # CMake 配置
│
└── build/output/                 # 编译输出（一键部署）
    ├── char_driver.ko           # ✅ ARM64 驱动模块
    ├── test_chardev             # ✅ ARM64 测试程序
    └── test_char_device.sh      # ✅ 测试脚本
```

## 📚 技术细节

### 驱动架构

```
字符设备驱动 (char_driver.ko)
    │
    ├─ 设备初始化 (char_driver_init)
    │   ├─ 分配设备私有数据
    │   ├─ 分配 4KB 内存缓冲区
    │   ├─ 动态分配设备号 (alloc_chrdev_region)
    │   ├─ 初始化字符设备 (cdev_init)
    │   ├─ 添加字符设备 (cdev_add)
    │   ├─ 创建设备类 (class_create)
    │   └─ 创建设备节点 (device_create)
    │
    ├─ 文件操作 (file_operations)
    │   ├─ open    → char_dev_open
    │   ├─ release → char_dev_release
    │   ├─ read    → char_dev_read
    │   ├─ write   → char_dev_write
    │   ├─ llseek  → char_dev_llseek
    │   └─ ioctl   → char_dev_ioctl
    │
    └─ 设备清理 (char_driver_exit)
        ├─ 删除设备节点
        ├─ 删除设备类
        ├─ 删除字符设备
        ├─ 释放设备号
        └─ 释放内存
```

### 并发控制

驱动使用 `mutex` 保护共享数据，支持多进程安全访问：

```c
struct char_device_data {
    char *buffer;           // 4KB 内存缓冲区
    size_t buffer_size;     // 缓冲区大小
    size_t data_size;       // 当前数据大小
    struct mutex lock;      // 互斥锁 ⭐
    struct cdev cdev;       // 字符设备结构
};
```

### 支持的操作

| 系统调用 | 驱动函数 | 功能描述 |
|---------|---------|---------|
| `open()` | `char_dev_open()` | 打开设备，获取设备私有数据 |
| `close()` | `char_dev_release()` | 关闭设备，释放资源 |
| `read()` | `char_dev_read()` | 从设备读取数据 |
| `write()` | `char_dev_write()` | 向设备写入数据 |
| `lseek()` | `char_dev_llseek()` | 改变文件位置 |
| `ioctl()` | `char_dev_ioctl()` | 设备控制命令 |

## 🎓 学习要点

### 1. 字符设备与块设备的区别

| 特性 | 字符设备 | 块设备 |
|------|---------|--------|
| 访问方式 | 字节流（顺序） | 块（随机） |
| 缓冲 | 无需缓冲 | 需要缓冲 |
| 示例 | 串口、传感器、本驱动 | 硬盘、SD卡 |
| 复杂度 | 较简单 | 较复杂 |

### 2. 设备号管理

```c
// 动态分配设备号（推荐）
alloc_chrdev_region(&dev_num, 0, 1, "mychardev");

// 静态分配设备号（不推荐）
register_chrdev_region(MKDEV(245, 0), 1, "mychardev");

// 查看设备号
cat /proc/devices | grep mychardev
```

### 3. 自动设备节点创建

```c
// 传统方式: 手动 mknod
// 现代方式: udev 自动创建

// 1. 创建设备类
class_create(CLASS_NAME);

// 2. 创建设备节点（udev 会自动创建 /dev/mychardev）
device_create(char_class, NULL, dev_num, NULL, DEVICE_NAME);
```

## 🔧 扩展开发

### 修改缓冲区大小

```c
// 在 char_driver.c 中修改
#define BUFFER_SIZE (16384)  // 改为 16KB
```

### 添加多设备支持

```c
// 创建多个设备实例
#define NUM_DEVICES 4

for (int i = 0; i < NUM_DEVICES; i++) {
    device_create(char_class, NULL, MKDEV(major, i), NULL, 
                  "mychardev%d", i);
}
// 将创建 /dev/mychardev0, /dev/mychardev1, ...
```

### 添加非阻塞 I/O 支持

```c
// 在 read/write 函数中添加
if (filp->f_flags & O_NONBLOCK) {
    // 非阻塞模式处理
    if (没有数据)
        return -EAGAIN;
}
```

## 💡 实际应用场景

1. **传感器数据采集**: 读取温度、湿度、加速度等传感器
2. **GPIO 控制**: LED、电机、继电器控制
3. **串口通信**: UART 驱动
4. **自定义硬件协议**: SPI、I2C 设备
5. **调试接口**: 内核调试、日志输出
6. **虚拟设备**: 进程间通信缓冲区

## 📖 参考资料

- 详细文档: `src/char_driver/README.md`
- Linux 设备驱动程序 (LDD3)
- Linux Kernel Documentation: `/Documentation/driver-api/`

## ⚖️ 许可证

GPL v2

---

**提示**: 这是一个完整的、可用于生产的字符设备驱动示例。您可以基于此代码开发自己的字符设备驱动！

