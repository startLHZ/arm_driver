# Block Device Driver Makefile
# ARM64驱动编译配置

ARCH ?= arm64
CROSS_COMPILE ?= /home/huaizhenlv/630_v27/out/host/toolchain/arm-gnu-toolchain-12.2.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-

obj-m += block_driver.o

ccflags-y := -I$(src)/../../include

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
