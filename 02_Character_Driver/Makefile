obj-m := pcd.o
KERNEL_DIR=/lib/modules/$(shell uname -r)/build/

all:
	make -C $(KERNEL_DIR) M=$(shell pwd) modules
	$(CC) pcdTest.c -o pcdtest
clean:
	make -C $(KERNEL_DIR) M=$(shell pwd) clean
	@rm -f pcdtest
help:
	make -C $(KERNEL_DIR) M=$(shell pwd) help
