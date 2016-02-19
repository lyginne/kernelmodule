
ifneq ($(KERNELRELEASE),)
	
	obj-m := mymodule.o
	mymodule-objs := src/mymodule_device.o
	mymodule-objs += src/mymodule_main.o

	EXTRA_CFLAGS := -I$(src)/src/include

else

	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

endif
