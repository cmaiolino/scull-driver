DEBUG ?= y

ifeq ($(DEBUG),y)
 DEBFLAGS = -O -g -DDEBUG_MODE # "-O" expand inlines
else
 DEBFLAGS = -O2
endif

EXTRA_CFLAGS += $(DEBFLAGS)

# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
	scull-objs := devices.o fops.o main.o
	obj-m := scull.o#module target to be compiled


# Otherwise we were called directly from the command
# line; invoke the kernel build system. 
else

	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif

clean:
	rm -rf *.ko *.o *.cmd *.mod.c modules.order Module.symvers *.unsigned *.o.d .tmp_versions/
