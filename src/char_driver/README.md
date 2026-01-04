# 字符设备驱动 (Character Device Driver)

## 📝 简介

这是一个完整的 Linux 字符设备驱动示例，使用内存作为存储后端，演示了字符设备驱动的所有核心功能。

### 特性

- ✅ **完整的文件操作**: open, close, read, write, llseek
- ✅ **IOCTL 控制命令**: 清空缓冲区、查询状态
- ✅ **自动设备节点创建**: `/dev/mychardev`
- ✅ **多进程安全**: 使用互斥锁保护共享数据
- ✅ **虚拟存储**: 4KB 内存缓冲区
- ✅ **内核版本兼容**: 支持 Linux 5.x 和 6.x

## 🏗️ 架构设计

```
用户空间应用
    ↕️ (read/write/ioctl)
/dev/mychardev (设备节点)
    ↕️
字符设备驱动 (char_driver.ko)
    ↕️
内存缓冲区 (4KB)
```

### 数据结构

```c
struct char_device_data {
    char *buffer;           /* 4KB 内存缓冲区 */
    size_t buffer_size;     /* 缓冲区大小 */
    size_t data_size;       /* 当前数据大小 */
    struct mutex lock;      /* 互斥锁 */
    struct cdev cdev;       /* 字符设备结构 */
};
```

## 🚀 快速开始

### 1. 编译驱动

在项目根目录执行：

```bash
./build.sh
```

编译完成后，驱动模块位于：
- `build/src/char_driver/char_driver.ko`
- `output/char_driver.ko`（最终输出）

### 2. 加载驱动

```bash
# 方式1: 使用 insmod
sudo insmod output/char_driver.ko

# 方式2: 使用 modprobe（需要先安装）
sudo modprobe char_driver
```

### 3. 验证设备

```bash
# 检查设备节点
ls -l /dev/mychardev
# 输出: crw------- 1 root root 245, 0 Nov 10 10:00 /dev/mychardev

# 查看内核日志
dmesg | tail -5
# 输出:
# CharDev: Initializing character device driver
# CharDev: Device number allocated: MAJOR=245, MINOR=0
# CharDev: Device /dev/mychardev created successfully
# CharDev: Buffer size: 4096 bytes
```

### 4. 测试驱动

```bash
# 运行完整测试
./output/test_chardev

# 查看设备信息
./output/test_chardev -i

# 写入数据
./output/test_chardev -w "Hello World"

# 读取数据
./output/test_chardev -r

# 清空设备
./output/test_chardev -c
```

### 5. 卸载驱动

```bash
sudo rmmod char_driver
```

## 📖 详细使用

### 基本读写操作

#### C 语言示例

```c
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    // 打开设备
    int fd = open("/dev/mychardev", O_RDWR);
    
    // 写入数据
    const char *data = "Hello, Character Device!";
    write(fd, data, strlen(data));
    
    // 重置到开头
    lseek(fd, 0, SEEK_SET);
    
    // 读取数据
    char buffer[100] = {0};
    read(fd, buffer, sizeof(buffer));
    printf("读取到: %s\n", buffer);
    
    // 关闭设备
    close(fd);
    return 0;
}
```

#### Shell 示例

```bash
# 写入数据
echo "Hello from shell" > /dev/mychardev

# 读取数据
cat /dev/mychardev

# 使用 dd 命令
dd if=/dev/mychardev of=output.txt bs=1 count=100
```

### IOCTL 命令

驱动支持以下 ioctl 命令：

| 命令 | 代码 | 功能 |
|------|------|------|
| `CHARDEV_IOCRESET` | 0 | 清空缓冲区 |
| `CHARDEV_IOCGETSIZE` | 1 | 获取缓冲区总大小 |
| `CHARDEV_IOCGETUSED` | 2 | 获取已使用大小 |

#### 使用示例

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

### 文件定位 (lseek)

```c
int fd = open("/dev/mychardev", O_RDWR);

// SEEK_SET: 从文件开头定位
lseek(fd, 100, SEEK_SET);  // 定位到第 100 字节

// SEEK_CUR: 从当前位置定位
lseek(fd, 50, SEEK_CUR);   // 向前移动 50 字节

// SEEK_END: 从文件末尾定位
lseek(fd, 0, SEEK_END);    // 定位到文件末尾
```

## 🔧 高级用法

### 权限管理

默认情况下，设备节点只有 root 可以访问。要允许普通用户访问：

```bash
# 修改设备权限
sudo chmod 666 /dev/mychardev

# 或者创建 udev 规则（永久生效）
echo 'KERNEL=="mychardev", MODE="0666"' | sudo tee /etc/udev/rules.d/99-mychardev.rules
sudo udevadm control --reload-rules
```

### 性能优化

```c
// 1. 使用大缓冲区减少系统调用
char buffer[4096];
ssize_t bytes = read(fd, buffer, sizeof(buffer));

// 2. 使用 O_NONBLOCK 非阻塞模式
int fd = open("/dev/mychardev", O_RDWR | O_NONBLOCK);

// 3. 批量操作
for (int i = 0; i < 1000; i++) {
    write(fd, data, size);
}
```

### 错误处理

```c
int fd = open("/dev/mychardev", O_RDWR);
if (fd < 0) {
    perror("打开设备失败");
    switch (errno) {
        case ENOENT:
            printf("设备不存在，请加载驱动\n");
            break;
        case EACCES:
            printf("权限不足，请使用 sudo\n");
            break;
        default:
            printf("错误代码: %d\n", errno);
    }
    return -1;
}

ssize_t bytes = write(fd, data, size);
if (bytes < 0) {
    if (errno == ENOSPC) {
        printf("设备空间不足\n");
    }
}
```

## 🧪 测试用例

测试程序 `test_chardev` 包含以下测试：

1. **基本操作测试**: 打开/关闭设备
2. **写入测试**: 写入数据并验证返回值
3. **读取测试**: 读取数据并验证内容
4. **Lseek 测试**: 测试 SEEK_SET/CUR/END
5. **IOCTL 测试**: 测试所有 ioctl 命令
6. **边界测试**: 空设备读取、缓冲区溢出
7. **压力测试**: 100 次连续读写
8. **并发测试**: 多进程同时访问

### 运行测试

```bash
# 运行所有测试
./output/test_chardev

# 查看设备信息
./output/test_chardev -i

# 显示帮助
./output/test_chardev -h
```

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

...

========================================
   测试结果统计
========================================
总测试数: 20
通过: 20
失败: 0
成功率: 100.0%

🎉 所有测试通过！
```

## 📊 技术细节

### 驱动架构

```
char_driver_init()
  ├─ 分配设备私有数据 (kzalloc)
  ├─ 分配内存缓冲区 (4KB)
  ├─ 动态分配设备号 (alloc_chrdev_region)
  ├─ 初始化字符设备 (cdev_init)
  ├─ 添加字符设备 (cdev_add)
  ├─ 创建设备类 (class_create)
  └─ 创建设备节点 (device_create) → /dev/mychardev
```

### 文件操作流程

```
用户调用 open() → char_dev_open()
  ├─ 获取设备私有数据
  ├─ 保存到 file->private_data
  └─ 打印日志

用户调用 write() → char_dev_write()
  ├─ 获取互斥锁
  ├─ 检查写入位置和大小
  ├─ copy_from_user() 复制数据
  ├─ 更新文件位置和数据大小
  └─ 释放互斥锁

用户调用 read() → char_dev_read()
  ├─ 获取互斥锁
  ├─ 检查读取位置和大小
  ├─ copy_to_user() 复制数据
  ├─ 更新文件位置
  └─ 释放互斥锁
```

### 并发控制

驱动使用 `mutex` 保护共享数据：

```c
// 初始化
mutex_init(&dev_data->lock);

// 在每个操作中
if (mutex_lock_interruptible(&dev_data->lock))
    return -ERESTARTSYS;
    
// ... 访问共享数据 ...

mutex_unlock(&dev_data->lock);
```

### 内存管理

```c
// 分配
char_dev_data = kzalloc(sizeof(*char_dev_data), GFP_KERNEL);
char_dev_data->buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);

// 释放（按相反顺序）
kfree(char_dev_data->buffer);
kfree(char_dev_data);
```

## 🐛 调试技巧

### 1. 查看内核日志

```bash
# 实时查看
sudo dmesg -w

# 过滤字符设备相关
dmesg | grep CharDev

# 查看最后 20 条
dmesg | tail -20
```

### 2. 增加调试输出

在驱动代码中添加：

```c
pr_debug("CharDev: Current pos=%lld, size=%zu\n", *f_pos, dev_data->data_size);
```

编译时启用调试：

```bash
# 在 Makefile 中添加
ccflags-y += -DDEBUG
```

### 3. 使用 strace 跟踪系统调用

```bash
strace -e trace=open,read,write,ioctl ./test_chardev
```

### 4. 检查设备状态

```bash
# 查看设备号
cat /proc/devices | grep mychardev

# 查看已加载模块
lsmod | grep char_driver

# 查看模块信息
modinfo char_driver.ko
```

## 🔍 常见问题

### 1. 设备节点未创建

**问题**: `/dev/mychardev` 不存在

**解决**:
```bash
# 检查驱动是否加载
lsmod | grep char_driver

# 查看内核日志
dmesg | tail

# 手动创建设备节点（临时方案）
sudo mknod /dev/mychardev c 245 0
sudo chmod 666 /dev/mychardev
```

### 2. 权限被拒绝

**问题**: `Permission denied`

**解决**:
```bash
# 使用 sudo
sudo ./test_chardev

# 或修改设备权限
sudo chmod 666 /dev/mychardev
```

### 3. 设备繁忙

**问题**: `Device or resource busy`

**解决**:
```bash
# 查看哪个进程在使用
lsof /dev/mychardev

# 强制卸载
sudo rmmod -f char_driver
```

### 4. 编译错误

**问题**: 内核头文件找不到

**解决**:
```bash
# 检查内核头文件
ls -l dependencies/kernel/include/

# 确保 KERNEL_DIR 正确
export KERNEL_DIR=/path/to/kernel
./build.sh
```

## 📚 扩展学习

### 相关概念

- **主设备号 (Major)**: 标识设备驱动程序
- **次设备号 (Minor)**: 标识具体设备实例
- **cdev**: 内核字符设备结构
- **file_operations**: 文件操作函数表
- **udev**: 自动设备节点管理

### 实战应用场景

1. **传感器驱动**: 读取温度、加速度等传感器数据
2. **GPIO 控制**: 控制 LED、电机等硬件
3. **串口设备**: UART 通信驱动
4. **自定义协议**: 实现特定的硬件通信协议
5. **虚拟设备**: 进程间通信、数据缓存

### 进阶修改建议

```c
// 1. 动态缓冲区大小
module_param(buffer_size, int, 0644);

// 2. 多设备支持
#define NUM_DEVICES 4
for (i = 0; i < NUM_DEVICES; i++) {
    // 创建 /dev/mychardev0, /dev/mychardev1, ...
}

// 3. 添加 poll 支持
static unsigned int char_dev_poll(struct file *filp, poll_table *wait) {
    // 实现异步通知
}

// 4. 添加 mmap 支持
static int char_dev_mmap(struct file *filp, struct vm_area_struct *vma) {
    // 实现内存映射
}
```

## 📄 许可证

GPL v2

## 👤 作者

Linux Driver Developer

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

