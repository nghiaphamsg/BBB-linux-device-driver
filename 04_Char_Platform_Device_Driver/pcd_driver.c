/*
 * @brief: pseudo character device driver to support four pseudo character devices.
 *         Implement open/release/read/write/lseek driver methods to handle user requests.
 * @author: NghiaPham
 * @ver: v0.4
 * @date: 2020/09/26
 *
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mod_devicetable.h>
#include "platform.h"

#undef pr_fmt
#define pr_fmt(fmt) "[%s]: " fmt, __func__

#define CLASS_NAME      "pcd_class"
#define DEV_NAME        "pcdevs"
#define NO_OF_DEVICES   4

/* Create dummy device configure */
enum pcdev_name {
    PCDEV_A_CONF,
    PCDEV_B_CONF,
    PCDEV_C_CONF,
    PCDEV_D_CONF,
};

struct device_configure {
    int configure_num1;
    int configure_num2;
};

struct device_configure pcdev_configure[] = {
    [PCDEV_A_CONF] = {.configure_num1 = 40, .configure_num2 = 254 },
    [PCDEV_B_CONF] = {.configure_num1 = 50, .configure_num2 = 253 },
    [PCDEV_C_CONF] = {.configure_num1 = 60, .configure_num2 = 252 },
    [PCDEV_D_CONF] = {.configure_num1 = 70, .configure_num2 = 251 },
};

struct platform_device_id pcdevs_ids[] = {
    [0] = {
        .name = "pcdev-Ax",
        .driver_data = PCDEV_A_CONF
    },
    [1] = {
        .name = "pcdev-Bx",
        .driver_data = PCDEV_B_CONF
    },
    [2] = {
        .name = "pcdev-Cx",
        .driver_data = PCDEV_C_CONF
    },
    [3] = {
        .name = "pcdev-Dx",
        .driver_data = PCDEV_D_CONF
    },
    {}
};

/* Structure represents device private data */
struct pcdev_private_data {
    struct pcdev_platform_data pdata;
    dev_t dev_num;
    char *buffer;
    struct cdev cdev;
};

/* Structure represents driver private data */
struct pcdrv_private_data {
    int total_device;
    dev_t device_number_base;
    struct class *class_pcd;
    struct device *device_pcd;
};
struct pcdrv_private_data pcdrv_data;

/* The prototype functions for the file operations of character driver */
int check_permission(int permission, int access_mode);
int pcd_open(struct inode *inode, struct file *filp);
int pcd_release(struct inode *inode, struct file *filp);
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos);
loff_t pcd_lseek(struct file *filp, loff_t offset, int whence);

/* The prototype functions for the platform driver */
int pcd_platform_driver_probe(struct platform_device *pdev);
int pcd_platform_driver_remove(struct platform_device *pdev);

struct file_operations pcd_fops = {
    .open = pcd_open,
    .write = pcd_write,
    .read = pcd_read,
    .release = pcd_release,
    .llseek = pcd_lseek,
    .owner = THIS_MODULE
};

struct platform_driver pcd_platform_driver = {
    .probe = pcd_platform_driver_probe,
    .remove = pcd_platform_driver_remove,
    /* Then try to match against the id table */
    .id_table = pcdevs_ids,
    /* fall-back to driver name match */
    .driver = {
        .name = "pseudo-char-device"
    }
};

int check_permission(int permission, int access_mode){

    if (permission == RDWR)
        return 0;

    /* Read only access */
    if ((permission == RDONLY) && ((access_mode & FMODE_READ) && !(access_mode & FMODE_WRITE)))
        return 0;

    /* Write only access */
    if ((permission == WRONLY) && ((access_mode & FMODE_WRITE) && !(access_mode & FMODE_READ)))
        return 0;
    
    return -EPERM;
}

int pcd_open(struct inode *inode, struct file *filp) {

    int minor_no, ret;
    struct pcdev_private_data *pcdev_data;

    /* Find out on on which device file open was attempted by user space */
    minor_no = MINOR(inode->i_rdev);

    /* Get device's private data structure */
    pcdev_data = container_of(inode->i_cdev, struct pcdev_private_data, cdev);

    /* Supply device private data to other method of the driver */
    filp->private_data = pcdev_data;

    /* Check permission */
    ret = check_permission(pcdev_data->pdata.permission, filp->f_mode);
    if (!ret)
        pr_info("Open was successful\n");
    else
        pr_info("Open was unsuccessful\n");

    return ret;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos) {

    int max_size;
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;
    max_size = pcdev_data->pdata.size;

    pr_info("Read requested for %zu bytes \n",count);
    pr_info("Current file position = %lld\n",*f_pos);

    /* Ajust the count argument */
    if ((*f_pos + count) > max_size)
        count = max_size - *f_pos;

    if (copy_to_user(buff, pcdev_data->buffer, count))
        return -EFAULT;
    
    /* Update current file position */
    *f_pos += count;
    pr_info("Number of bytes successfully read = %zu\n", count);
    pr_info("Updated file position = %lld\n",*f_pos);

    return count;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos) {

    int max_size;
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;
    max_size = pcdev_data->pdata.size;

    pr_info("Write requested %zu bytes \n",count);
    pr_info("Current file position = %lld\n",*f_pos);

    /* Ajust the count argument */
    if ((*f_pos + count) > max_size)
        count = max_size - *f_pos;

    if (!count)
        return -ENOMEM;

    if (copy_from_user(pcdev_data->buffer, buff, count))
        return -EFAULT;

    /* Update current file position */
    *f_pos += count;
    pr_info("Number of bytes successfully written = %zu\n", count);
    pr_info("Updated file position = %lld\n",*f_pos);

    return count;
}

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence) {
    
    loff_t temp;
    unsigned int max_size;
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;
    max_size = pcdev_data->pdata.size;

    switch (whence) {
        case SEEK_SET:
            if ((offset > max_size) || (offset < 0))
                return -EINVAL;
            filp->f_pos = offset;
            break;
        case SEEK_CUR:
            temp = filp->f_pos + offset;
            if ((temp > max_size) || (temp < 0))
                return -EINVAL;
            filp->f_pos = temp;
            break;
        case SEEK_END:
            temp = max_size + offset;
            if ((temp > max_size) || (temp < 0))
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

int pcd_platform_driver_probe(struct platform_device *pdev) {

    int ret;
    struct pcdev_private_data *dev_data;
    struct pcdev_platform_data *pdata;

    pr_info("Device was detected\n");

    /* Get platform data */
    pdata = (struct pcdev_platform_data *)dev_get_platdata(&pdev->dev);
    if (!pdata) {
        pr_info("No platform data available\n");
        return -EINVAL;
    }

    /* Dynamically allocate memory for the device private data */
    dev_data = devm_kzalloc(&pdev->dev, sizeof(*dev_data), GFP_KERNEL);
    if (!dev_data) {
        pr_info("Cannot allocate memory\n");
        return -ENOMEM;
    }

    /* Save device private data pointer in platform device structure (release)*/
    dev_set_drvdata(&pdev->dev, dev_data);

    dev_data->pdata.size = pdata->size;
    dev_data->pdata.permission = pdata->permission;
    dev_data->pdata.serial_number = pdata->serial_number;

    pr_info("Device size %d\n", dev_data->pdata.size);
    pr_info("Device permission %d\n", dev_data->pdata.permission);
    pr_info("Device serial number %s\n", dev_data->pdata.serial_number);

    pr_info("Configure item 1: %d\n", pcdev_configure[pdev->id_entry->driver_data].configure_num1);
    pr_info("Configure item 2: %d\n", pcdev_configure[pdev->id_entry->driver_data].configure_num2);

    /* Dynamically allocate memory for the device buffer */
    dev_data->buffer = devm_kzalloc(&pdev->dev, dev_data->pdata.size, GFP_KERNEL);
    if (!dev_data->buffer) {
        pr_info("Cannot allocate memory\n");
        return -ENOMEM;
    }

    dev_data->dev_num = pcdrv_data.device_number_base + pdev->id;

    cdev_init(&dev_data->cdev, &pcd_fops);
    dev_data->cdev.owner = THIS_MODULE;
    ret = cdev_add(&dev_data->cdev, dev_data->dev_num, 1);
    if (ret < 0) {
        pr_info("Cdev add failed\n");
        return ret;
    }

    /* Create device file for the detected platform device */
    pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, NULL, dev_data->dev_num, NULL, "pcdev-%d", pdev->id);
    if (IS_ERR(pcdrv_data.device_pcd)) {
        ret = PTR_ERR(pcdrv_data.device_pcd);
        cdev_del(&dev_data->cdev);
        return ret;
    }

    pcdrv_data.total_device++;

    pr_info("Probe was successful\n");
    pr_info("--------------------\n");
    return 0;
}

int pcd_platform_driver_remove(struct platform_device *pdev) {

    struct pcdev_private_data *dev_data = dev_get_drvdata(&pdev->dev);

    device_destroy(pcdrv_data.class_pcd, dev_data->dev_num);
    cdev_del(&dev_data->cdev);
    pcdrv_data.total_device--;

    pr_info("Device was removed");
    return 0;
}


static int __init char_platform_driver_init(void) {

    int ret;

    /* Dynamically allocate a device number <one device> */
    ret = alloc_chrdev_region(&pcdrv_data.device_number_base, 0, NO_OF_DEVICES, DEV_NAME);
    if (ret < 0)
        goto error_out;

    /* Create class and device files </sys/class/...> */
    pcdrv_data.class_pcd = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(pcdrv_data.class_pcd)) {
        ret = PTR_ERR(pcdrv_data.class_pcd);
        goto unregister_allocate_dd;
    }

    ret = platform_driver_register(&pcd_platform_driver);
    if (ret < 0)
        goto class_del;

    pr_info("Platform driver module loaded\n");

    return 0;

class_del:
    class_destroy(pcdrv_data.class_pcd);
    unregister_chrdev_region(pcdrv_data.device_number_base, NO_OF_DEVICES);
unregister_allocate_dd:
    unregister_chrdev_region(pcdrv_data.device_number_base, NO_OF_DEVICES);
error_out:
    return ret;

}

static void __exit char_platform_driver_exit(void) {
    platform_driver_unregister(&pcd_platform_driver);
    class_destroy(pcdrv_data.class_pcd);
    unregister_chrdev_region(pcdrv_data.device_number_base, NO_OF_DEVICES);

    pr_info("Platform driver module unloaded\n");
}

module_init(char_platform_driver_init);
module_exit(char_platform_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NghiaPham");
MODULE_DESCRIPTION("Platform driver & platform deivce");
MODULE_INFO(board,"Beaglebone Black rev.c");