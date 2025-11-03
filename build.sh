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

# 检查本地依赖是否存在
if [ ! -d "$TOOLCHAIN_PATH" ]; then
    echo "=========================================="
    echo "错误: 本地工具链不存在！"
    echo "=========================================="
    echo ""
    echo "路径: $TOOLCHAIN_PATH"
    echo ""
    echo "请确保 dependencies/ 目录存在并包含工具链。"
    echo "如果是新克隆的项目，需要先导入依赖。"
    echo ""
    exit 1
fi

if [ ! -d "$KERNEL_DIR" ] || [ ! -f "$KERNEL_DIR/Makefile" ]; then
    echo "=========================================="
    echo "错误: 本地内核目录不存在或无效！"
    echo "=========================================="
    echo ""
    echo "路径: $KERNEL_DIR"
    echo ""
    echo "请确保 dependencies/ 目录存在并包含内核头文件。"
    echo "如果是新克隆的项目，需要先导入依赖。"
    echo ""
    exit 1
fi

echo ""
echo "=========================================="
echo "ARM驱动编译配置"
echo "=========================================="
echo "架构:           $ARCH"
echo "交叉编译器:     $CROSS_COMPILE"
echo "内核路径:       $KERNEL_DIR"
echo "依赖模式:       本地依赖 (独立模式)"
echo "=========================================="
echo ""

# 清理旧的编译
rm -rf build
mkdir -p build
cd build

# 配置CMake
cmake -DKERNEL_DIR="$KERNEL_DIR" \
      -DARCH="$ARCH" \
      -DCROSS_COMPILE="$CROSS_COMPILE" \
      .. || exit 1

# 编译
make || exit 1

echo ""
echo "正在编译内核模块..."
make modules || exit 1

# 创建清晰的输出目录
echo ""
echo "整理输出文件..."
rm -rf output
mkdir -p output

# 复制驱动模块
find src -name "*.ko" -exec cp {} output/ \; 2>/dev/null
# 复制测试程序
find examples -name "test_app" -type f -executable -exec cp {} output/ \; 2>/dev/null

echo ""
echo "=========================================="
echo "✓ 编译完成！"
echo "=========================================="

if [ -d output ] && [ "$(ls -A output 2>/dev/null)" ]; then
    echo ""
    echo "📦 生成的文件 (build/output/):"
    echo "----------------------------------------"
    for file in output/*; do
        if [ -f "$file" ]; then
            filename=$(basename "$file")
            size=$(ls -lh "$file" | awk '{print $5}')
            filetype=$(file -b "$file" | cut -d',' -f1)
            printf "  ✓ %-25s %8s  [%s]\n" "$filename" "$size" "$filetype"
        fi
    done
    echo "----------------------------------------"
    
    echo ""
    echo "💡 快速访问:"
    echo "  cd output/           # 直接进入输出目录"
    echo "  ls output/           # 查看生成的文件"
    echo "  scp output/*.ko ...  # 直接传输驱动"
    echo ""
    echo "部署方法:"
    echo "  1. 传输: scp output/*.ko user@device:/path/"
    echo "  2. 加载: sudo insmod <驱动名>.ko"
    echo "  3. 卸载: sudo rmmod <模块名>"
else
    echo "⚠ 警告: output目录为空"
fi
echo "=========================================="
