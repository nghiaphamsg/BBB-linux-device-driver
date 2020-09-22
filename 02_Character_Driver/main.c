#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define CHAR_NAME       "pcd"
#define MEM_SIZE        512

char device_buffer[MEM_SIZE];

dev_t device_number;

struct cdev pcd_cdev;

struct file_operations pcd_fops;

static int __init char_device_driver_init(void) {

    /* Dynamically allocate a device number */
    alloc_chrdev_region(&device_number, 0, 1, CHAR_NAME);

    /* Make a character device registration with the VFS */
    cdev_init(&pcd_cdev, &pcd_fops);
    pcd_cdev.owner = THIS_MODULE;
    cdev_add(&pcd_cdev, device_number, 1);


    return 0;
}

static void __exit char_device_driver_exit(void) {

}

module_init(char_device_driver_init);
module_exit(char_device_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NghiaPham");
MODULE_DESCRIPTION("Simple character driver to deal with a pseudo character device");
MODULE_INFO(board,"Beaglebone black rev.c");