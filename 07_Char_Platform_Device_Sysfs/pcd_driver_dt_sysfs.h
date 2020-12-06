/*
 * @brief: General structure platform device & driver 
 * @author: NghiaPham
 * @ver: v0.1
 * @date: 2020/12/05
 *
*/

#ifndef PCD_DRIVER_DT_SYSFS_H
#define PCD_DRIVER_DT_SYSFS_H

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
#define ATTR_GP_NAME    "pcd_attr_gp"

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

/* The prototype functions for device attributes */
ssize_t max_size_show(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t max_size_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
ssize_t serial_number_show(struct device *dev, struct device_attribute *attr, char *buf);

/* Other sub-functions */
struct pcdev_platform_data* pcdev_check_pf_dt(struct device *dev);
int pcd_sysfs_create(struct device *dev);

#endif // PCD_DRIVER_DT_SYSFS_H