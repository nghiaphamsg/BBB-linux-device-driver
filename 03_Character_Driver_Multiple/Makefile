obj-m := pcd_multiple.o
KERNEL_DIR=/lib/modules/$(shell uname -r)/build/

all:
	make -C $(KERNEL_DIR) M=$(shell pwd) modules
	$(CC) pcd_mul_test.c -o pcd_mul_test
clean:
	make -C $(KERNEL_DIR) M=$(shell pwd) clean
	@rm -f pcd_mul_test
help:
	make -C $(KERNEL_DIR) M=$(shell pwd) help
