/*
 * @brief: General structure platform device & driver 
 * @author: NghiaPham
 * @ver: v0.1
 * @date: 2020/12/07
 *
*/

#ifndef GPIO_SYSFS_H
#define GPIO_SYSFS_H

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
#include <linux/gpio/consumer.h>

#undef pr_fmt
#define pr_fmt(fmt) "[%s]: " fmt, __func__

#define CLASS_NAME      "bone_gpio_class"

/* Structure represents device private data */
struct gpiodev_private_data {
    char lable[20];
    struct gpio_desc *gpio_desc;

};

/* Structure represents driver private data */
struct gpiodrv_private_data {
    int total_device;
    struct class *class_gpio;
    struct device **dev;
};

/* The prototype functions for the platform driver */
int gpio_sysfs_probe(struct platform_device *pdev);
int gpio_sysfs_remove(struct platform_device *pdev);


#endif // GPIO_SYSFS_H