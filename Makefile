obj-m := loop.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
default:
		$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
		$(MAKE) -C $(KDIR) M=$(PWD) clean

