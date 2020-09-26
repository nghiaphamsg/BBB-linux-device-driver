/*
 * @brief: Write a character driver to deal with a pseudo character device (pcd).
 *         The pseudo-device is a memory buffer of some size. The driver what you
 *         write must support reading, writing and seeking to this device
 *         Test the driver functionality by running user-level.
 * @author: NghiaPham
 * @ver: v0.3
 * @date: 2020/09/23
 *
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

#undef pr_fmt
#define pr_fmt(fmt) "[%s]: " fmt, __func__

#define CHAR_NAME       "pcd_devices"
#define CLASS_NAME      "pcd_class"
#define DEV_NAME        "pcd"

#define MEM_SIZE        512

char device_buffer[MEM_SIZE];

dev_t device_number;

struct cdev pcd_cdev = {
    .owner = THIS_MODULE
};
struct class *class_pcd;
struct device *device_pcd;

/* The prototype functions for the character driver -- must come before the struct definition */
int pcd_open(struct inode *inode, struct file *filp);
int pcd_release(struct inode *inode, struct file *filp);
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos);
loff_t pcd_lseek(struct file *filp, loff_t offset, int whence);

struct file_operations pcd_fops = {
    .open = pcd_open,
    .write = pcd_write,
    .read = pcd_read,
    .release = pcd_release,
    .llseek = pcd_lseek,
    .owner = THIS_MODULE
};

int pcd_open(struct inode *inode, struct file *filp) {
    pr_info("Opened successful\n");
    return 0;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos) {

    /* Ajust the count argument */
    if ((*f_pos + count) > MEM_SIZE)
        count = MEM_SIZE - *f_pos;

    if (copy_to_user(buff, device_buffer, count))
        return -EFAULT;
    
    /* Update current file position */
    *f_pos += count;
    pr_info("Number of bytes successfully read = %zu\n", count);

    return count;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos) {

    /* Ajust the count argument */
    if ((*f_pos + count) > MEM_SIZE)
        count = MEM_SIZE - *f_pos;

    if (!count)
        return -ENOMEM;

    if (copy_from_user(device_buffer, buff, count))
        return -EFAULT;

    /* Update current file position */
    *f_pos += count;
    pr_info("Number of bytes successfully written = %zu\n", count);

    return count;
}

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence) {

    loff_t temp;

    switch (whence) {
        case SEEK_SET:
            if ((offset > MEM_SIZE) || (offset < 0))
                return -EINVAL;
            filp->f_pos = offset;
            break;
        case SEEK_CUR:
            temp = filp->f_pos + offset;
            if ((temp > MEM_SIZE) || (temp < 0))
                return -EINVAL;
            filp->f_pos = temp;
            break;
        case SEEK_END:
            temp = MEM_SIZE + offset;
            if ((temp > MEM_SIZE) || (temp < 0))
                return -EINVAL;
            filp->f_pos = temp;
            break;
        default:
            return -EINVAL;
    }

    pr_info("New value of the file position = %lld\n",filp->f_pos);
    return filp->f_pos;
}

int pcd_release(struct inode *inode, struct file *filp) {
    pr_info("Released successful\n");
    return 0;
}

static int __init char_device_driver_init(void) {

    int ret;

    /* Dynamically allocate a device number <one device> */
    ret = alloc_chrdev_region(&device_number, 0, 1, CHAR_NAME);
    if (ret < 0)
        goto error_out;
    pr_info("Device number <Major>:<Minor> = %d:%d\n", MAJOR(device_number), MINOR(device_number));

    /* Make a character device registration with the VFS */
    cdev_init(&pcd_cdev, &pcd_fops);
    ret = cdev_add(&pcd_cdev, device_number, 1);
    if (ret < 0)
        goto unregister_char_dd;

    /* Create class and device files </sys/class/...> */
    class_pcd = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(class_pcd)) {
        ret = PTR_ERR(class_pcd);
        goto cdev_del;
    }

    device_pcd = device_create(class_pcd, NULL, device_number, NULL, DEV_NAME);
    if (IS_ERR(device_pcd)) {
        ret = PTR_ERR(device_pcd);
        goto class_del;
    }

    pr_info("Module init was successful\n");

    return 0;

class_del:
    class_destroy(class_pcd);
cdev_del:
    cdev_del(&pcd_cdev);
unregister_char_dd:
    unregister_chrdev_region(device_number, 1);
error_out:
    return ret;
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
MODULE_INFO(board,"Beaglebone Black rev.c");