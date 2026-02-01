# ARM 驱动开发项目

这是一个用于 ARM64 平台的 Linux 内核驱动开发项目，包含多种驱动示例和测试程序。

## 快速开始

### 0. 安装系统依赖（首次使用）

在运行脚本前，需要确保系统已安装必要的编译工具：

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install -y flex bison libssl-dev libelf-dev bc git make gcc
```

**CentOS/RHEL:**
```bash
sudo yum install -y flex bison openssl-devel elfutils-libelf-devel bc git make gcc
```

**Fedora:**
```bash
sudo dnf install -y flex bison openssl-devel elfutils-libelf-devel bc git make gcc
```

**Arch Linux:**
```bash
sudo pacman -S flex bison openssl libelf bc git make gcc
```

> **提示**：`setup_kernel.sh` 脚本会自动检查这些依赖，如果缺失会给出安装提示。

### 1. 配置内核源码

首次使用前，需要下载并配置内核源码：

```bash
./setup_kernel.sh
```

该脚本会自动：
- **检查系统依赖**，如有缺失会提示安装命令
- 从 GitHub 克隆 Linux 内核源码（版本 6.1.134）
- 配置交叉编译环境
- 准备模块编译所需的构建工具
- 创建符号链接到 `dependencies/kernel` 目录

> **注意**：内核源码会下载到 `dependencies/kernel_src/` 目录，已在 `.gitignore` 中排除，不会被提交到仓库。

### 2. 编译驱动和测试程序

配置完内核源码后，运行：

```bash
./build.sh
```

编译产物会输出到 `build/output/` 目录：
- `*.ko` - 内核驱动模块
- 测试程序（可执行文件）

## 项目结构

```
arm_driver/
├── setup_kernel.sh          # 内核源码配置脚本（首次使用必须运行）
├── build.sh                 # 驱动编译脚本
├── dependencies/
│   ├── kernel_src/          # 内核源码（由 setup_kernel.sh 创建，不提交到仓库）
│   ├── kernel/              # 指向 kernel_src 的符号链接
│   └── toolchain/           # ARM64 交叉编译工具链
├── src/                     # 驱动源码
│   ├── simple_driver/       # 简单字符设备驱动
│   ├── char_driver/         # 字符设备驱动
│   ├── block_driver/        # 块设备驱动
│   ├── net_block_driver/    # 网络块设备驱动
│   └── i2c_driver/          # I2C 驱动
├── examples/                # 测试程序
└── include/                 # 公共头文件
```

## 驱动模块说明

### 1. Simple Driver
基础的字符设备驱动示例，演示基本的驱动注册和设备文件操作。

### 2. Character Driver
完整的字符设备驱动，支持读写操作。

### 3. Block Driver
块设备驱动示例，模拟一个内存块设备。

### 4. I2C Driver
I2C 总线驱动示例，演示如何与 I2C 设备通信。

### 5. Network Block Driver
网络块设备驱动，可通过网络访问块设备。

## 交叉编译说明

本项目使用 ARM64 交叉编译工具链：
- 工具链：`aarch64-none-linux-gnu-gcc 12.2.1`
- 目标架构：`arm64`
- 内核版本：`Linux 6.1.134`

## 常见问题

### Q: 运行 setup_kernel.sh 提示缺少工具怎么办？
**A:** 脚本会自动检测缺失的工具并显示安装命令。按照提示安装即可：
```bash
# Ubuntu/Debian
sudo apt install -y flex bison libssl-dev libelf-dev bc git make gcc
```

### Q: 首次编译提示 "内核源码未配置或不完整"
**A:** 请先运行 `./setup_kernel.sh` 配置内核源码。

### Q: setup_kernel.sh 下载很慢怎么办？
**A:** 可以编辑 `setup_kernel.sh`，将 `KERNEL_REPO` 修改为国内镜像：
```bash
KERNEL_REPO="https://mirrors.tuna.tsinghua.edu.cn/git/linux.git"
```

### Q: 如何切换到其他内核版本？
**A:** 编辑 `setup_kernel.sh` 中的 `KERNEL_VERSION` 变量，然后重新运行该脚本。

### Q: 内核源码占用空间太大
**A:** 脚本默认使用浅克隆（`--depth 1`），只下载指定版本，不包含完整历史记录，已经是最小化的。如果仍然觉得太大，可以在编译完成后删除 `dependencies/kernel_src/.git/` 目录。

## 开发环境要求

- 操作系统：Linux（已在 Ubuntu/Debian 上测试）
- 必需工具：`git`, `make`, `cmake`, `gcc`
- 磁盘空间：约 2-3GB（用于内核源码）

## 许可证

请参考各驱动源码文件中的许可证声明。
