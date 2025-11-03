#!/bin/bash

#=============================================================================
# ARM驱动编译脚本 - 本地依赖模式（独立）
#=============================================================================

# 获取项目根目录
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"

# ============ 配置区域 ============
# ARM架构配置
ARCH=arm64

# 使用本地依赖
TOOLCHAIN_PATH="$PROJECT_ROOT/dependencies/toolchain/bin"
KERNEL_DIR="$PROJECT_ROOT/dependencies/kernel"

CROSS_COMPILE=${TOOLCHAIN_PATH}/aarch64-none-linux-gnu-
export CC=${TOOLCHAIN_PATH}/aarch64-none-linux-gnu-gcc
# ===================================


# 清理旧的编译
rm -rf build
mkdir -p build
cd build

# 配置CMake
cmake -DKERNEL_DIR="$KERNEL_DIR" \
      -DARCH="$ARCH" \
      -DCROSS_COMPILE="$CROSS_COMPILE" \
      .. || exit 1

# 编译测试程序和内核模块
make || exit 1

# 创建清晰的输出目录
rm -rf output
mkdir -p output

# 复制所有驱动模块
find src -name "*.ko" -exec cp {} output/ \; 2>/dev/null

# 复制所有测试程序
find examples -type f -executable -exec cp {} output/ \; 2>/dev/null

# 复制测试脚本
find examples -name "*.sh" -exec cp {} output/ \; 2>/dev/null
