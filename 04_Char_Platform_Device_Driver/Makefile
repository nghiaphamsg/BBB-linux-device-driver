obj-m := pcd_device_setup.o pcd_driver.o
KERNEL_DIR=/lib/modules/$(shell uname -r)/build/

all:
	make -C $(KERNEL_DIR) M=$(shell pwd) modules
clean:
	make -C $(KERNEL_DIR) M=$(shell pwd) clean
help:
	make -C $(KERNEL_DIR) M=$(shell pwd) help
