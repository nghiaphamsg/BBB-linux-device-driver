/*
 * @brief: pseudo character device driver to support four pseudo character devices.
 *         Implement open/release/read/write/lseek driver methods to handle user requests.
 * @author: NghiaPham
 * @ver: v0.2
 * @date: 2020/10/18
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
#include <linux/of.h>
#include <linux/of_device.h>
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

/* Struct used for matching with setup code */
struct platform_device_id pcdevs_ids[] = {
    {.name = "pcdev-Ax", .driver_data = PCDEV_A_CONF},
    {.name = "pcdev-Bx", .driver_data = PCDEV_B_CONF},
    {.name = "pcdev-Cx", .driver_data = PCDEV_C_CONF},
    {.name = "pcdev-Dx", .driver_data = PCDEV_D_CONF},
    {}
};

/* Struct used for matching a device tree (am335x_boneblack_ldd.dtsi) */
struct of_device_id org_pcdev_dt_match[] = {
    {.compatible = "pcdev-Ax", .data = (void *)PCDEV_A_CONF},
    {.compatible = "pcdev-Bx", .data = (void *)PCDEV_B_CONF},
    {.compatible = "pcdev-Cx", .data = (void *)PCDEV_C_CONF},
    {.compatible = "pcdev-Dx", .data = (void *)PCDEV_D_CONF},
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
        .name = "pseudo-char-device",
        .of_match_table = org_pcdev_dt_match
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

/* This function check device from device tree or setup code */
struct pcdev_platform_data* pcdev_check_pf_dt(struct device *dev) {
    struct device_node *dev_node = dev->of_node;
    struct pcdev_platform_data *pdata;

    if (!dev_node) {
        /* This probe didn't happen beacause of device tree node */
        return NULL;
    }

    pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
    if (!pdata) {
        dev_info(dev, "Cannot allocate memory\n");
        return ERR_PTR(-ENOMEM);
    }

    if (of_property_read_string(dev_node, "org,device-serial-num", &pdata->serial_number)) {
        dev_info(dev, "Missing serial number property\n");
        return ERR_PTR(-EINVAL);
    }
    if (of_property_read_u32(dev_node, "org,size", &pdata->size)) {
        dev_info(dev, "Missing size property\n");
        return ERR_PTR(-EINVAL);
    }
    if (of_property_read_u32(dev_node, "org,permission", &pdata->permission)) {
        dev_info(dev, "Missing permission property\n");
        return ERR_PTR(-EINVAL);
    }
    
    return pdata;
}

int pcd_platform_driver_probe(struct platform_device *pdev) {

    int ret, driver_data;
    struct pcdev_private_data *dev_data;
    struct pcdev_platform_data *pdata;
    struct device *dev = &pdev->dev;
    
    dev_info(dev, "Device was detected\n");

    pdata = pcdev_check_pf_dt(dev);
    if (IS_ERR(pdata))
        return PTR_ERR(pdata);

    /* Get platform data */
    if (!pdata) {
        /* Device setup by code */
        pdata = (struct pcdev_platform_data *)dev_get_platdata(dev);
        if (!pdata) {
            dev_info(dev, "No platform data available\n");
            return -EINVAL;
        }
        driver_data = pdev->id_entry->driver_data;
    } else {
        /* Devie tree */
        pr_info("Dt start");
        driver_data = (int) of_device_get_match_data(dev);
    }

    /* Dynamically allocate memory for the device private data */
    dev_data = devm_kzalloc(dev, sizeof(*dev_data), GFP_KERNEL);
    if (!dev_data) {
        dev_info(dev, "Cannot allocate memory\n");
        return -ENOMEM;
    }

    /* Save device private data pointer in platform device structure (release)*/
    dev_set_drvdata(dev, dev_data);

    dev_data->pdata.size = pdata->size;
    dev_data->pdata.permission = pdata->permission;
    dev_data->pdata.serial_number = pdata->serial_number;

    pr_info("Device size %d\n", dev_data->pdata.size);
    pr_info("Device permission %d\n", dev_data->pdata.permission);
    pr_info("Device serial number %s\n", dev_data->pdata.serial_number);

    pr_info("Configure item 1: %d\n", pcdev_configure[driver_data].configure_num1);
    pr_info("Configure item 2: %d\n", pcdev_configure[driver_data].configure_num2);

    /* Dynamically allocate memory for the device buffer */
    dev_data->buffer = devm_kzalloc(dev, dev_data->pdata.size, GFP_KERNEL);
    if (!dev_data->buffer) {
        dev_info(dev, "Cannot allocate memory\n");
        return -ENOMEM;
    }

    dev_data->dev_num = pcdrv_data.device_number_base + pcdrv_data.total_device;

    cdev_init(&dev_data->cdev, &pcd_fops);
    dev_data->cdev.owner = THIS_MODULE;
    ret = cdev_add(&dev_data->cdev, dev_data->dev_num, 1);
    if (ret < 0) {
        dev_info(dev, "Cdev add failed\n");
        return ret;
    }

    /* Create device file for the detected platform device */
    pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, dev, dev_data->dev_num, NULL, "pcdev-%d", pcdrv_data.total_device);
    if (IS_ERR(pcdrv_data.device_pcd)) {
        ret = PTR_ERR(pcdrv_data.device_pcd);
        cdev_del(&dev_data->cdev);
        return ret;
    }

    pcdrv_data.total_device++;

    dev_info(dev, "Probe was successful\n");
    pr_info("--------------------\n");
    return 0;
}

int pcd_platform_driver_remove(struct platform_device *pdev) {

    struct pcdev_private_data *dev_data = dev_get_drvdata(&pdev->dev);

    device_destroy(pcdrv_data.class_pcd, dev_data->dev_num);
    cdev_del(&dev_data->cdev);
    pcdrv_data.total_device--;

    dev_info(&pdev->dev, "Device was removed");
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
MODULE_DESCRIPTION("Pseudo character device which handles multiple devices");
MODULE_INFO(board,"Beaglebone Black rev.c");