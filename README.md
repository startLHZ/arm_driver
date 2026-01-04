# Linux ARM Driver Framework

ARM64驱动开发框架 - 完全独立，一键编译

## ✓ 项目特点

- **完全独立**: 所有依赖都在项目内，不依赖外部目录
- **架构**: ARM64 (aarch64)
- **工具链**: GNU 12.2.1（本地）
- **内核**: 本地内核头文件
- **状态**: ✓ 可随意移动和分享

## 🚀 快速开始

### 编译驱动

```bash
./build.sh
```

就这么简单！

### 2️⃣ 获取生成的文件

编译完成后，所有生成的文件都在 `output/` 目录：

```bash
ls output/
# simple_driver.ko  - ARM64驱动模块
# test_app          - ARM64测试程序
```

### 3️⃣ 部署到ARM设备

```bash
# 传输驱动
scp output/*.ko user@device:/tmp/

# 在设备上加载
sudo insmod simple_driver.ko

# 查看日志
dmesg | tail

# 卸载驱动
sudo rmmod simple_driver
```

## 📁 目录结构

```
linux_arm_driver/
├── build.sh                     # 一键编译脚本
├── README.md                    # 本文档
├── BLOCK_DEVICE_USAGE.md       # 块设备驱动详细指南
│
├── src/                         # 驱动源码目录
│   ├── simple_driver/          # 简单驱动示例
│   │   ├── simple_driver.c     # 驱动源码
│   │   ├── Makefile           # 驱动编译配置
│   │   ├── CMakeLists.txt     # CMake配置
│   │   └── README.md          # 驱动说明文档
│   │
│   └── block_driver/           # 块设备驱动
│       ├── block_driver.c     # 驱动源码
│       ├── Makefile          # 驱动编译配置
│       ├── CMakeLists.txt    # CMake配置
│       └── README.md         # 驱动说明文档
│
├── examples/                    # 测试程序目录
│   ├── simple_driver/          # simple_driver测试程序
│   │   ├── test_app.c         # 测试程序源码
│   │   └── CMakeLists.txt     # CMake配置
│   │
│   └── block_driver/           # block_driver测试程序
│       ├── test_block.c       # 测试程序源码
│       ├── test_block_device.sh  # 自动化测试脚本
│       └── CMakeLists.txt     # CMake配置
│
├── include/                     # 公共头文件
│   ├── arm_driver.h            # 驱动公共头文件
│   └── block_driver.h          # 块设备头文件
│
├── dependencies/                # 本地依赖
│   ├── toolchain/              # ARM64交叉编译工具链
│   └── kernel/                 # 内核头文件
│
├── build/                       # 编译中间文件
│   ├── src/
│   │   ├── simple_driver/     # simple_driver构建输出
│   │   │   └── simple_driver.ko
│   │   └── block_driver/      # block_driver构建输出
│   │       └── block_driver.ko
│   └── examples/
│       ├── simple_driver/     # test_app构建输出
│       │   └── test_app
│       └── block_driver/      # test_block构建输出
│           ├── test_block
│           └── test_block_device.sh
│
└── output/                      # 🎯 最终生成的文件
    ├── simple_driver.ko        # 简单驱动模块
    ├── block_driver.ko         # 块设备驱动模块
    ├── char_driver.ko          # 字符设备驱动模块
    ├── test_app                # simple_driver测试程序
    ├── test_block              # block_driver测试程序
    ├── test_chardev            # char_driver测试程序
    └── test_char_device.sh     # char_driver自动化脚本
```

**💡 提示**: 每个驱动都有独立的目录，便于管理和扩展！

## 🎯 驱动列表

### 1. Simple Driver
- **位置**: `src/simple_driver/`
- **说明**: 基础驱动示例，演示内核模块基本结构
- **文档**: [src/simple_driver/README.md](src/simple_driver/README.md)

### 2. Block Device Driver
- **位置**: `src/block_driver/`
- **说明**: 完整的块设备驱动，支持ext4文件系统
- **特性**: 
  - 基于 blk-mq 框架
  - 支持格式化和挂载
  - 512MB虚拟存储
  - 可自定义后端存储
- **文档**: 
  - [src/block_driver/README.md](src/block_driver/README.md)
  - [BLOCK_DEVICE_USAGE.md](BLOCK_DEVICE_USAGE.md) - 详细使用指南

### 3. Character Device Driver
- **位置**: `src/char_driver/`
- **说明**: 虚拟字符设备驱动，使用内存作为存储后端
- **特性**: 
  - 支持 read/write/llseek/ioctl 操作
  - 自动创建设备节点 `/dev/mychardev`
  - 多进程并发安全（互斥锁保护）
  - 4KB 内存缓冲区
  - 完整的测试程序和自动化脚本
- **文档**: 
  - [src/char_driver/README.md](src/char_driver/README.md)
  - [CHAR_DEVICE_USAGE.md](CHAR_DEVICE_USAGE.md) - 快速使用指南

## ➕ 添加新驱动

### 方法 1: 按照现有结构添加（推荐）

1. 创建驱动目录和文件：
```bash
mkdir -p src/my_new_driver
mkdir -p examples/my_new_driver
```

2. 创建驱动源码 `src/my_new_driver/my_driver.c`

3. 创建 `src/my_new_driver/Makefile`:
```makefile
ARCH ?= arm64
CROSS_COMPILE ?= ...

obj-m += my_driver.o
ccflags-y := -I$(src)/../../include

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
```

4. 创建 `src/my_new_driver/CMakeLists.txt`:
```cmake
configure_file(Makefile ${CMAKE_CURRENT_BINARY_DIR}/Makefile COPYONLY)
configure_file(my_driver.c ${CMAKE_CURRENT_BINARY_DIR}/my_driver.c COPYONLY)
```

5. 在 `src/CMakeLists.txt` 添加：
```cmake
add_subdirectory(my_new_driver)
```

6. 创建测试程序和对应的 CMakeLists.txt（参考 examples/simple_driver/）

7. 运行 `./build.sh`

8. 在 `build/src/my_new_driver/` 找到 `my_driver.ko`

### 方法 2: 快速添加（简单驱动）

如果只是简单的驱动，可以：
1. 在 `src/` 任一驱动目录创建 `.c` 文件
2. 修改该目录的 Makefile 添加新的 `obj-m`
3. 运行 `./build.sh`

## � 修改配置

如需修改架构或工具链配置，编辑 `build.sh` 的配置区域（第11-17行）：

```bash
# ARM架构配置
ARCH=arm64    # 可改为 arm, x86 等

# 工具链和内核路径（相对于项目根目录）
TOOLCHAIN_PATH="$PROJECT_ROOT/dependencies/toolchain/bin"
KERNEL_DIR="$PROJECT_ROOT/dependencies/kernel"
```

**项目已优化为独立模式，可以随意移动和分享！🚀**## 目录结构

- `src/` - 驱动源码
- `include/` - 头文件  
- `examples/` - 测试程序
- `build.sh` - **一键编译脚本（自动配置）**
- `快速开始.md` - 详细使用说明

## 编译方式

```bash
# 方式1: 自动查找内核（推荐，已在build.sh中配置路径）
./build.sh

# 方式2: 临时指定内核路径
KERNEL_DIR=~/linux ./build.sh
```

## Load/Unload Modules

```bash
sudo insmod src/simple_driver.ko
sudo insmod src/char_driver.ko
sudo rmmod simple_driver
sudo rmmod char_driver
```

## Test

```bash
./examples/test_app
```
