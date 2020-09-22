/*
 * @brief: Write a character driver to deal with a pseudo character device (pcd).
 *         The pseudo-device is a memory buffer of some size. The driver what you
 *         write must support reading, writing and seeking to this device
 *         Test the driver functionality by running user-level.
 * @author: NghiaPham 
 * @ver: v0.1 
 * @date: 2020/09/23
 *
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt, __func__

#define CHAR_NAME       "pcd_devices"
#define CLASS_NAME      "pcd_class"
#define DEV_NAME        "pcd"

#define MEM_SIZE        512

char device_buffer[MEM_SIZE];

dev_t device_number;

struct cdev pcd_cdev;

struct class *class_pcd;

struct device *device_pcd;

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos) {

    return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos) {

    return 0;
}

int pcd_open(struct inode *inode, struct file *filp) {

    return 0;
}

int pcd_release(struct inode *inode, struct file *filp) {

    return 0;
}

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence) {

    return 0;
}

struct file_operations pcd_fops = {
    .open = pcd_open,
    .write = pcd_write,
    .read = pcd_read,
    .release = pcd_release,
    .llseek = pcd_lseek,
    .owner = THIS_MODULE
};

static int __init char_device_driver_init(void) {

    /* Dynamically allocate a device number <one device> */
    alloc_chrdev_region(&device_number, 0, 1, CHAR_NAME);
    pr_info("Device number <Major>:<Minor> = %d:%d\n", MAJOR(device_number), MINOR(device_number));

    /* Make a character device registration with the VFS */
    cdev_init(&pcd_cdev, &pcd_fops);
    pcd_cdev.owner = THIS_MODULE;
    cdev_add(&pcd_cdev, device_number, 1);

    /* Create class and device files </sys/class/...> */
    class_pcd = class_create(THIS_MODULE, CLASS_NAME);
    device_pcd = device_create(class_pcd, NULL, device_number, NULL, DEV_NAME);

    pr_info("Module init was successful\n");    

    return 0;
}

static void __exit char_device_driver_exit(void) {

	device_destroy(class_pcd, device_number);
	class_destroy(class_pcd);
	cdev_del(&pcd_cdev);
	unregister_chrdev_region(device_number, 1);
	pr_info("Module unloaded\n");
}

module_init(char_device_driver_init);
module_exit(char_device_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NghiaPham");
MODULE_DESCRIPTION("Simple character driver to deal with a pseudo character device");
MODULE_INFO(board,"Beaglebone black rev.c");