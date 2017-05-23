#
# Makefile for a kernel module
#
# 2017, Wolfhard Prell (wolfhard.prell@gmail.com)
#
ifneq ($(KERNELRELEASE),)
obj-m := register_cdev.o

else 
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

endif