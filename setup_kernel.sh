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
# 内核版本（根据你当前的 version.h 推断）
KERNEL_VERSION="6.1.134"
KERNEL_MAJOR="6.1"

# 内核源码仓库
KERNEL_REPO="https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git"
# 或者使用 GitHub 镜像（通常更快）
# KERNEL_REPO="https://github.com/torvalds/linux.git"

# ARM64 架构配置
ARCH=arm64
TOOLCHAIN_PATH="$PROJECT_ROOT/dependencies/toolchain/bin"
CROSS_COMPILE=${TOOLCHAIN_PATH}/aarch64-none-linux-gnu-
# ===================================

echo "=========================================="
echo "内核源码下载和配置工具"
echo "=========================================="
echo "目标版本: Linux $KERNEL_VERSION"
echo "架构: $ARCH"
echo "源码目录: $KERNEL_SRC_DIR"
echo "编译目录: $KERNEL_BUILD_DIR"
echo "=========================================="

# ============ 检查必需的系统工具 ============
echo ""
echo "🔍 检查系统依赖..."

MISSING_TOOLS=()
REQUIRED_TOOLS=(
    "git:Git 版本控制系统"
    "make:Make 构建工具"
    "gcc:GCC 编译器"
    "flex:词法分析器生成器"
    "bison:语法分析器生成器"
    "bc:计算器工具"
)

REQUIRED_PACKAGES=(
    "libssl-dev:OpenSSL 开发库"
    "libelf-dev:ELF 文件处理库"
)

# 检查命令行工具
for tool_info in "${REQUIRED_TOOLS[@]}"; do
    tool="${tool_info%%:*}"
    desc="${tool_info#*:}"
    if ! command -v "$tool" &> /dev/null; then
        MISSING_TOOLS+=("  - $tool ($desc)")
    fi
done

# 检查开发库（通过查找头文件）
if [ ! -f "/usr/include/openssl/ssl.h" ] && [ ! -f "/usr/local/include/openssl/ssl.h" ]; then
    MISSING_TOOLS+=("  - libssl-dev (OpenSSL 开发库)")
fi

if [ ! -f "/usr/include/libelf.h" ] && [ ! -f "/usr/local/include/libelf.h" ]; then
    MISSING_TOOLS+=("  - libelf-dev (ELF 文件处理库)")
fi

# 如果有缺失的工具，显示安装提示
if [ ${#MISSING_TOOLS[@]} -gt 0 ]; then
    echo ""
    echo "❌ 检测到缺少以下必需工具："
    echo ""
    for tool in "${MISSING_TOOLS[@]}"; do
        echo "$tool"
    done
    echo ""
    echo "=========================================="
    echo "请运行以下命令安装所需依赖："
    echo "=========================================="
    echo ""
    
    # 根据发行版提供不同的安装命令
    if command -v apt &> /dev/null; then
        echo "  sudo apt update"
        echo "  sudo apt install -y flex bison libssl-dev libelf-dev bc git make gcc"
    elif command -v yum &> /dev/null; then
        echo "  sudo yum install -y flex bison openssl-devel elfutils-libelf-devel bc git make gcc"
    elif command -v dnf &> /dev/null; then
        echo "  sudo dnf install -y flex bison openssl-devel elfutils-libelf-devel bc git make gcc"
    elif command -v pacman &> /dev/null; then
        echo "  sudo pacman -S flex bison openssl libelf bc git make gcc"
    else
        echo "  请根据你的发行版安装以上工具"
    fi
    
    echo ""
    echo "=========================================="
    echo ""
    
    # 询问是否继续
    read -p "是否已经安装完成，继续执行？(y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "已取消。请安装依赖后重新运行此脚本。"
        exit 1
    fi
    
    # 再次检查关键工具
    echo "重新检查依赖..."
    if ! command -v flex &> /dev/null || ! command -v bison &> /dev/null; then
        echo "❌ 错误: flex 或 bison 仍未安装，无法继续"
        exit 1
    fi
    echo "✅ 依赖检查通过"
else
    echo "✅ 所有必需工具已安装"
fi

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
    
    # 检查当前版本是否已经是目标版本
    CURRENT_VERSION=$(git describe --tags 2>/dev/null || echo "unknown")
    if [ "$CURRENT_VERSION" = "v$KERNEL_VERSION" ]; then
        echo "✅ 当前已经是目标版本 v$KERNEL_VERSION，跳过克隆"
    else
        echo "当前版本: $CURRENT_VERSION"
        echo "目标版本: v$KERNEL_VERSION"
        echo "正在更新到目标版本..."
        
        # 获取新的 tags
        git fetch --depth=1 origin tag "v$KERNEL_VERSION" 2>/dev/null || {
            echo "⚠️  获取特定版本失败，将删除旧仓库重新克隆..."
            cd "$PROJECT_ROOT"
            rm -rf "$KERNEL_SRC_DIR"
            
            echo "📦 重新克隆内核源码..."
            git clone --depth 1 --branch "v$KERNEL_VERSION" "$KERNEL_REPO" "$KERNEL_SRC_DIR" || {
                echo "❌ 克隆失败"
                exit 1
            }
            cd "$KERNEL_SRC_DIR"
        }
    fi
else
    echo ""
    echo "📦 正在克隆内核源码（这可能需要较长时间）..."
    echo "仓库: $KERNEL_REPO"
    
    # 创建父目录
    mkdir -p "$(dirname "$KERNEL_SRC_DIR")"
    
    # 浅克隆指定分支以节省时间和空间
    # 注意: 如果需要完整历史，移除 --depth 1 参数
    git clone --depth 1 --branch "v$KERNEL_VERSION" "$KERNEL_REPO" "$KERNEL_SRC_DIR" || {
        echo ""
        echo "⚠️  直接克隆特定版本失败，尝试完整克隆后切换版本..."
        git clone --depth 1 "$KERNEL_REPO" "$KERNEL_SRC_DIR"
        cd "$KERNEL_SRC_DIR"
        git fetch --depth=1 origin tag "v$KERNEL_VERSION"
        git checkout "v$KERNEL_VERSION"
    }
    
    cd "$KERNEL_SRC_DIR"
fi

# 确保我们在正确的版本上
echo ""
echo "📍 验证内核版本..."

CURRENT_VERSION=$(git describe --tags 2>/dev/null || echo "unknown")
if [ "$CURRENT_VERSION" = "v$KERNEL_VERSION" ]; then
    echo "✅ 当前版本: $CURRENT_VERSION"
else
    echo "尝试切换到版本 v$KERNEL_VERSION..."
    git checkout "v$KERNEL_VERSION" 2>/dev/null || {
        echo "⚠️  未找到精确版本 v$KERNEL_VERSION，尝试查找最接近的版本..."
        
        # 尝试获取可用的 tags
        git fetch --tags --depth=1 2>/dev/null || true
        
        # 获取 6.1.x 分支的最新稳定版
        AVAILABLE_VERSION=$(git tag -l "v${KERNEL_MAJOR}.*" 2>/dev/null | sort -V | tail -n 1)
        if [ -n "$AVAILABLE_VERSION" ]; then
            echo "使用版本: $AVAILABLE_VERSION"
            git checkout "$AVAILABLE_VERSION"
        else
            echo "⚠️  无法通过 git tag 找到版本，尝试直接检出..."
            # 浅克隆可能没有完整的 tag 信息，尝试直接使用当前状态
            CURRENT_VERSION=$(git describe --tags 2>/dev/null || echo "unknown")
            if [ "$CURRENT_VERSION" != "unknown" ]; then
                echo "✅ 使用当前版本: $CURRENT_VERSION"
            else
                echo "❌ 错误: 无法确定内核版本"
                exit 1
            fi
        fi
    }
fi

# 2. 配置内核用于外部模块编译
echo ""
echo "⚙️  配置内核编译环境..."

# 清理之前的配置（如果有）
make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE mrproper

# 使用默认配置
echo "使用 defconfig 生成默认配置..."
make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE defconfig

# 3. 准备模块编译所需的基础环境
echo ""
echo "🔨 准备模块编译环境..."
make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE modules_prepare

# 4. 编译脚本工具（modpost 等）
echo ""
echo "🔧 编译内核脚本工具..."
make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE scripts

# 5. 创建符号链接到旧的 kernel 目录（可选，用于兼容）
echo ""
echo "🔗 创建构建目录链接..."
if [ -d "$KERNEL_BUILD_DIR" ] && [ ! -L "$KERNEL_BUILD_DIR" ]; then
    echo "备份旧的 kernel 目录..."
    mv "$KERNEL_BUILD_DIR" "${KERNEL_BUILD_DIR}.backup.$(date +%Y%m%d_%H%M%S)"
fi

# 移除旧链接（如果存在）
rm -f "$KERNEL_BUILD_DIR"

# 创建符号链接
ln -sf "$KERNEL_SRC_DIR" "$KERNEL_BUILD_DIR"

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

