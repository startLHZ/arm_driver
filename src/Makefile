# ARM64驱动编译配置
ARCH ?= arm64
CROSS_COMPILE ?= /home/huaizhenlv/630_v27/out/host/toolchain/arm-gnu-toolchain-12.2.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-

obj-m += simple_driver.o
# obj-m += char_driver.o  # 如果有其他驱动，去掉注释

ccflags-y := -I$(src)/../include
