obj-m := pcd_device_setup.o pcd_driver.o
KERNEL_DIR=/lib/modules/$(shell uname -r)/build/

all default: modules

modules modules_install help clean:
	make -C $(KERNEL_DIR) M=$(shell pwd) $@
