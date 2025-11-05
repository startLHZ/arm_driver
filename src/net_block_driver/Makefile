# Makefile for Network Block Device Driver

obj-m += net_block_driver.o

KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	make -C $(KDIR) M=$(PWD) modules_install

load:
	sudo insmod net_block_driver.ko

unload:
	sudo rmmod net_block_driver

.PHONY: all clean install load unload
