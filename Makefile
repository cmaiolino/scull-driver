# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.

ifneq ($(KERNELRELEASE),)
	scull-objs := devices.o fops.o
	obj-m := scull.o#module target to be compiled


# Otherwise we were called directly from the command
# line; invoke the kernel build system. 
else

	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

default:
	#make -C /lib/modules/2.6.34.7-56.fc13.i686.PAE/build M=/home/cmaiolino/Sources/LDD3/ modules
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif

clean:
	rm -rf scull.ko devices.o fops.o scull.mod.c scull.mod.o scull.o modules.order Module.symvers scull.ko.unsigned .devices.o.cmd .fops.o.d .scull.ko.cmd .scull.ko.unsigned.cmd .scull.mod.o.cmd .scull.o.cmd .scull.o.d .tmp_versions/
