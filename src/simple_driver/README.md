# Simple Driver

一个简单的 Linux 内核驱动示例程序。

## 📝 功能说明

这是一个基础的字符设备驱动，用于演示 Linux 内核模块的基本结构和开发流程。

## 📂 文件结构

```
src/simple_driver/
├── simple_driver.c    # 驱动源代码
├── Makefile          # 驱动编译配置
├── CMakeLists.txt    # CMake 构建配置
└── README.md         # 本文档

examples/simple_driver/
├── test_app.c        # 测试程序源代码
└── CMakeLists.txt    # CMake 构建配置
```

## 🔧 编译

### 方法 1: 使用项目构建脚本（推荐）

```bash
cd /home/huaizhenlv/linux_arm_driver
./build.sh
```

编译产物：
- 驱动模块：`build/src/simple_driver/simple_driver.ko`
- 测试程序：`build/examples/simple_driver/test_app`

### 方法 2: 单独编译驱动

```bash
cd src/simple_driver
export KERNEL_DIR=/path/to/kernel
make
```

## 🚀 使用方法

### 1. 加载驱动

```bash
sudo insmod build/src/simple_driver/simple_driver.ko
```

### 2. 查看驱动信息

```bash
# 查看加载的模块
lsmod | grep simple_driver

# 查看内核日志
dmesg | tail
```

### 3. 运行测试程序

```bash
cd build/examples/simple_driver
./test_app
```

### 4. 卸载驱动

```bash
sudo rmmod simple_driver
```

## 📚 相关文档

- 项目根目录 README.md - 项目总体说明
- include/arm_driver.h - 驱动头文件

## 🐛 故障排查

如果遇到问题，请：
1. 检查内核日志：`dmesg | grep simple`
2. 确认模块已加载：`lsmod | grep simple_driver`
3. 检查内核版本是否匹配

## 📄 许可证

GPL v2
