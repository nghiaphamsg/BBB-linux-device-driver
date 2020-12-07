/*
 * @brief: Implement GPIO sysfs & create custom attributes of a device.
 * @author: NghiaPham
 * @ver: v0.1
 * @date: 2020/12/07
 *
*/

#include "gpio_sysfs.h"

/* Struct used for matching a device tree */
struct of_device_id gpio_dt_match[] = {
    {.compatible = "org,bone-gpio-sysfs"},
    {}
};

struct gpiodrv_private_data gpiodrv_data;

struct platform_driver gpio_platform_driver = {
    .probe = gpio_sysfs_probe,
    .remove = gpio_sysfs_remove,
    /* fall-back to driver name match */
    .driver = {
        .name = "bone-gpio-sysfs",
        .of_match_table = of_match_ptr(gpio_dt_match)
    }
};

int gpio_sysfs_probe(struct platform_device *pdev) {

    const char *name;
    int count = 0, ret;

    struct gpiodev_private_data *dev_data; 
    struct device *dev = &pdev->dev;

    /* Associated device tree node */
    struct device_node *parent = pdev->dev.of_node;
    struct device_node *child = NULL;

    /* Find the next available child node */
    for_each_available_child_of_node(parent, child) {
        dev_data = devm_kzalloc(dev, sizeof(*dev_data), GFP_KERNEL);
        if (!dev_data) {
            dev_err(dev, "Can't allocate memory\n");
            return -ENOMEM;
        }

        /* Get a lable device tree child node */
        if (of_property_read_string(child, "lable", &name)) {
            dev_warn(dev, "Missing lable from device tree\n");
            snprintf(dev_data->lable, sizeof(dev_data->lable), "unkn_gpio%d", count);
        } 
        else {
            strcpy(dev_data->lable, name);
            dev_info(dev, "GPIO lable = %s", dev_data->lable);
        }

        /* Get a GPIO descriptor from a device's child node */
        dev_data->gpio_desc = devm_fwnode_get_gpiod_from_child(dev, "bone", &child->fwnode, GPIOD_ASIS, dev_data->lable);
        if (IS_ERR(dev_data->gpio_desc)) {
            ret = PTR_ERR(dev_data->gpio_desc);
            if (ret == -ENOENT)
                dev_err(dev, "No GPIO has been assigned to the requested function and/or index\n");
            return ret;
        }
        
        /* Set GPIO direction to output */
        ret = gpiod_direction_output(dev_data->gpio_desc, 0);
        if (ret) {
            dev_err(dev, "GPIO direction set failed\n");
            return ret;
        }

        count++;
    }
    return 0;
}

int gpio_sysfs_remove(struct platform_device *pdev) {
    return 0;
}

static int __init gpio_sysfs_init(void) {

    int ret;

    /* Create class and device files </sys/class/...> */
    gpiodrv_data.class_gpio = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(gpiodrv_data.class_gpio)) {
        ret = PTR_ERR(gpiodrv_data.class_gpio);
    }

    ret = platform_driver_register(&gpio_platform_driver);
    if (ret < 0)
        goto class_del;
    pr_info("Platform driver module loaded\n");

class_del:
    class_destroy(gpiodrv_data.class_gpio);
    
    return 0;
}

static void __exit gpio_sysfs_exit(void) {

    platform_driver_unregister(&gpio_platform_driver);
    class_destroy(gpiodrv_data.class_gpio);

    pr_info("Platform driver module unloaded\n");
}


module_init(gpio_sysfs_init);
module_exit(gpio_sysfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NghiaPham");
MODULE_DESCRIPTION("GPIO exercise");
MODULE_INFO(board,"Beaglebone Black rev.c");