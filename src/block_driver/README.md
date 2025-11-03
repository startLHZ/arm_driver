# Flash Block Device Driver

基于 Linux 内核 blk-mq 框架的 Flash 块设备驱动，通过 I2C 访问 Serial NOR Flash。

## 📝 功能说明

- ✅ 创建块设备 `/dev/flashblk`
- ✅ 支持 ext4 文件系统格式化和挂载
- ✅ 通过 I2C 访问 Serial NOR Flash（786KB）
- ✅ 提供 sysfs 接口直接读取任意 Flash 地址
- ✅ 标准文件系统操作（读/写/目录等）

## 📂 文件结构

```
src/block_driver/
├── block_driver.c     # Flash 块设备驱动源码
├── Makefile          # 驱动编译配置
└── CMakeLists.txt    # CMake 构建配置

examples/block_driver/
├── test_block.c      # 块设备测试程序
├── read_flash.c      # Flash 数据读取工具
└── CMakeLists.txt    # CMake 构建配置
```

## 🔧 编译

```bash
./build.sh
```

编译产物：
- `build/output/block_driver.ko` - 驱动模块
- `build/output/test_block` - 块设备测试程序
- `build/output/read_flash` - Flash 读取工具

## 🚀 使用方法

### 1. 加载驱动

```bash
sudo insmod block_driver.ko
ls -l /dev/flashblk  # 验证设备创建
```

### 2. 读取 Flash 数据（sysfs 接口）

**方法1：命令行（最简单）**
```bash
# 读取 0x0C0003 地址的 32 字节
echo 0x0C0003 32 > /sys/class/flashblk/flashblk/flash_data
cat /sys/class/flashblk/flashblk/flash_data
```

**方法2：使用 read_flash 工具**
```bash
./read_flash 0x0C0003 16   # 读取 16 字节
./read_flash 0x000000 256  # 读取 256 字节
```

### 3. 使用块设备（可选）

```bash
# 格式化为 ext4
sudo mkfs.ext4 /dev/flashblk

# 挂载
sudo mkdir -p /mnt/flashblk
sudo mount /dev/flashblk /mnt/flashblk

# 读写文件
echo "test" > /mnt/flashblk/file.txt
cat /mnt/flashblk/file.txt

# 卸载
sudo umount /mnt/flashblk
```

### 4. 卸载驱动

```bash
sudo rmmod block_driver
```

## ⚙️ Sysfs 接口说明

所有接口位于：`/sys/class/flashblk/flashblk/`

| 节点 | 功能 | 读/写 |
|------|------|-------|
| `flash_addr` | Flash 地址 | 读写 |
| `flash_len` | 读取长度（1-256字节）| 读写 |
| `flash_data` | 读取数据 | 读写 |

**输出格式**（十六进制，每行16字节）：
```
Flash Read: addr=0x000C0003 len=16

0000: 48 65 6C 6C 6F 20 57 6F 72 6C 64 00 00 00 00 00
```

## ⚙️ 配置参数

修改 `block_driver.c` 中的参数：

```c
#define FLASH_I2C_BUS 4         // I2C 总线号
#define FLASH_I2C_ADDR 0x11     // I2C 设备地址
#define FLASH_DATA_SIZE 786813  // Flash 数据大小
#define FLASH_START_ADDR 0x0    // Flash 起始地址
```

## 🐛 故障排查

```bash
# 检查模块是否加载
lsmod | grep block_driver

# 查看内核日志
dmesg | grep flashblk

# 检查 sysfs 接口
ls -l /sys/class/flashblk/flashblk/

# 重新加载驱动
sudo rmmod block_driver
sudo insmod block_driver.ko
```

**常见错误码**：
- `-5 (EIO)`: I2C 通信失败，检查硬件连接
- `-19 (ENODEV)`: I2C 适配器不存在
- `-22 (EINVAL)`: 参数无效

## 📚 技术要点

- **blk-mq 框架**：多队列块设备架构
- **I2C Flash 访问**：通过 I2C 读取 Serial NOR Flash
- **Sysfs 接口**：提供用户空间直接访问 Flash 的调试接口
- **Mutex 锁**：保证 Flash I/O 与块设备操作互斥

## 📄 许可证

GPL v2
