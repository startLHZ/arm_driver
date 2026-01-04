# Makefile for Character Device Driver

# 默认架构和交叉编译器（会被 build.sh 覆盖）
ARCH ?= arm64
CROSS_COMPILE ?= aarch64-linux-gnu-

# 内核目录（会被 build.sh 覆盖）
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build

# 驱动模块名称
obj-m += char_driver.o

# 编译选项：添加包含路径
ccflags-y := -I$(src)/../../include

# 默认目标
all:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_DIR) M=$(PWD) modules

# 清理
clean:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_DIR) M=$(PWD) clean

# 安装模块（本地测试用）
install:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_DIR) M=$(PWD) modules_install

.PHONY: all clean install

