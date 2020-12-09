obj-m := pcd.o
KERNEL_DIR=/lib/modules/$(shell uname -r)/build/

all default: modules

modules modules_install help clean:
	make -C $(KERNEL_DIR) M=$(shell pwd) $@
