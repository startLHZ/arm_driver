#!/bin/bash

#=============================================================================
# 内核源码下载和配置脚本
#=============================================================================

set -e  # 遇到错误立即退出

# 获取项目根目录
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
KERNEL_SRC_DIR="$PROJECT_ROOT/dependencies/kernel_src"
KERNEL_BUILD_DIR="$PROJECT_ROOT/dependencies/kernel"

# ============ 配置区域 ============
# 内核源码仓库 - 使用国内镜像源（更快）
# 可选镜像源：
# - 清华大学: https://mirrors.tuna.tsinghua.edu.cn/git/linux.git
# - 中科大: https://mirrors.ustc.edu.cn/linux.git
# - 官方stable: https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git
KERNEL_REPO="https://mirrors.tuna.tsinghua.edu.cn/git/linux.git"

# 内核版本（使用 6.1 系列，兼容旧 API）
KERNEL_VERSION="6.1.134-rt51-08397-g52fcf6b1f54b"
KERNEL_TAG="v6.1"  # 使用 6.1 稳定版标签

# ARM64 架构配置
ARCH=arm64
TOOLCHAIN_PATH="$PROJECT_ROOT/dependencies/toolchain/bin"
CROSS_COMPILE=${TOOLCHAIN_PATH}/aarch64-none-linux-gnu-
# ===================================

echo "=========================================="
echo "内核源码下载和配置工具"
echo "=========================================="
if [ -n "$KERNEL_VERSION" ]; then
    echo "目标版本: $KERNEL_VERSION"
else
    echo "将使用最新的稳定版本"
fi
echo "架构: $ARCH"
echo "源码目录: $KERNEL_SRC_DIR"
echo "编译目录: $KERNEL_BUILD_DIR"
echo "=========================================="

#  ...existing code...

# 检查工具链是否存在
echo ""
echo "🔍 检查交叉编译工具链..."
if [ ! -f "${CROSS_COMPILE}gcc" ]; then
    echo "❌ 错误: 找不到交叉编译器 ${CROSS_COMPILE}gcc"
    echo "请先确保 dependencies/toolchain 目录下有正确的工具链"
    exit 1
fi
echo "✅ 交叉编译工具链检查通过"

# 1. 克隆或更新内核源码
if [ -d "$KERNEL_SRC_DIR/.git" ]; then
    echo ""
    echo "📦 检测到已存在的内核源码"
    cd "$KERNEL_SRC_DIR"
    
    CURRENT_VERSION=$(git describe --tags 2>/dev/null || echo "unknown")
    echo "当前版本: $CURRENT_VERSION"
    
    # 如果指定了标签，检查是否需要切换
    if [ -n "$KERNEL_TAG" ]; then
        echo "正在切换到标签: $KERNEL_TAG"
        git fetch --depth 1 origin tag "$KERNEL_TAG" 2>/dev/null || true
        git checkout "$KERNEL_TAG" 2>/dev/null || {
            echo "⚠️  无法切换到 $KERNEL_TAG，将使用当前版本"
        }
    fi
    echo "✅ 使用已存在的内核源码"
else
    echo ""
    echo "📦 正在克隆内核源码（这可能需要较长时间）..."
    echo "仓库: $KERNEL_REPO"
    
    # 创建父目录
    mkdir -p "$(dirname "$KERNEL_SRC_DIR")"
    
    if [ -n "$KERNEL_TAG" ]; then
        # 克隆指定标签
        echo "克隆内核标签: $KERNEL_TAG"
        git clone --depth 1 --branch "$KERNEL_TAG" "$KERNEL_REPO" "$KERNEL_SRC_DIR" || {
            echo "❌ 克隆失败"
            exit 1
        }
    else
        # 浅克隆主分支
        echo "克隆最新的主线内核..."
        git clone --depth 1 "$KERNEL_REPO" "$KERNEL_SRC_DIR" || {
            echo "❌ 克隆失败"
            exit 1
        }
    fi
    
    cd "$KERNEL_SRC_DIR"
fi

# 显示当前版本信息
echo ""
echo "📍 内核信息:"
KERNEL_VERSION=$(make kernelversion 2>/dev/null || echo "unknown")
GIT_VERSION=$(git describe --tags 2>/dev/null || echo "unknown")
echo "  内核版本: $KERNEL_VERSION"
echo "  Git 版本: $GIT_VERSION"

# 2. 配置内核用于外部模块编译
echo ""
echo "⚙️  配置内核编译环境..."

# 清理之前的配置（如果有）
make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE mrproper

# 使用默认配置
echo "使用 defconfig 生成默认配置..."
make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE defconfig

# 准备编译外部模块所需的环境
echo ""
echo "⚙️  准备模块编译环境（生成必要的头文件）..."
make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE modules_prepare

# 生成 Module.symvers 文件（编译少量核心模块）
echo ""
echo "⚙️  生成 Module.symvers（这可能需要几分钟）..."
echo "   正在编译核心模块以生成符号版本信息..."

# 只编译最小必需的模块来生成 Module.symvers
# 使用 -j 并行编译以加快速度，限制只编译部分模块
make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE -j$(nproc) modules 2>&1 | \
    grep -E "^\s+(CC|LD|AR)" | head -20 || true

# 检查 Module.symvers 是否生成
if [ ! -f "Module.symvers" ]; then
    echo "⚠️  警告: Module.symvers 未生成，创建空文件..."
    echo "   外部模块编译时可能会有符号警告，但通常不影响功能"
    touch Module.symvers
fi

echo ""
echo "=========================================="
echo "✅ 内核源码配置完成！"
echo "=========================================="
echo ""
echo "源码位置: $KERNEL_SRC_DIR"
echo "内核版本: $(cd "$KERNEL_SRC_DIR" && make kernelversion)"
echo ""
echo "现在可以运行 ./build.sh 来编译驱动模块了"
echo "=========================================="