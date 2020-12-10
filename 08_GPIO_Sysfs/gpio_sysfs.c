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

ssize_t direction_show(struct device *dev, struct device_attribute *attr, char *buf) {
    
    int ret;
    char *direc;
    struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);

    ret = gpiod_get_direction(dev_data->gpio_desc);
    if (ret < 0)
        return ret;
    
    direc = (ret == 0) ? "out" : "in";
    return sprintf(buf, "%s\n", direc);
}

ssize_t value_show(struct device *dev, struct device_attribute *attr, char *buf) {

    int value;
    struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);

    value = gpiod_get_value(dev_data->gpio_desc);
    if (value < 0)
        return value;
    
    return sprintf(buf, "%d\n", value);
}

ssize_t lable_show(struct device *dev, struct device_attribute *attr, char *buf) {
    
    struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);
    return sprintf(buf, "%s\n", dev_data->lable);
}

ssize_t direction_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {

    int ret;
    struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);

    if (sysfs_streq(buf, "in"))
        ret = gpiod_direction_input(dev_data->gpio_desc);
    else if (sysfs_streq(buf, "out"))
        ret = gpiod_direction_output(dev_data->gpio_desc, 0);
    else
        ret = -EINVAL;

    return ret ? : count;
}

ssize_t value_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {

    int ret;
    long value;
    struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);

    ret = kstrtol(buf, 0, &value);
    if (ret)
        return ret;
    
    gpiod_set_value(dev_data->gpio_desc, value);
    return count;
}

static DEVICE_ATTR_RW(direction);
static DEVICE_ATTR_RW(value);
static DEVICE_ATTR_RO(lable);

/* Create list of attribute groups */
static struct attribute *gpio_attrs[] = {
    &dev_attr_direction.attr,
    &dev_attr_value.attr,
    &dev_attr_lable.attr,
    NULL
};

static struct attribute_group gpio_attr_group = {
    .attrs = gpio_attrs
};

static const struct attribute_group *gpio_attr_groups[] = 
{
    &gpio_attr_group,
    NULL
};

int gpio_sysfs_probe(struct platform_device *pdev) {

    const char *name;
    int pos = 0, ret;

    struct gpiodev_private_data *dev_data; 
    struct device *dev = &pdev->dev;

    /* Associated device tree node */
    struct device_node *parent = pdev->dev.of_node;
    struct device_node *child = NULL;

    gpiodrv_data.total_device = of_get_child_count(parent);
    if (!gpiodrv_data.total_device) {
        dev_err(dev, "No child node found\n");
        return -EINVAL;
    }
    dev_info(dev, "Total child node found = %d", gpiodrv_data.total_device);
    gpiodrv_data.dev = devm_kzalloc(dev, sizeof(struct device *) *gpiodrv_data.total_device, GFP_KERNEL);

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
            snprintf(dev_data->lable, sizeof(dev_data->lable), "unkn_gpio%d", pos);
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

        /* Creates a device and registers it with sysfs */
        gpiodrv_data.dev[pos] = device_create_with_groups(  gpiodrv_data.class_gpio,
                                                            dev,
                                                            0,
                                                            dev_data,
                                                            gpio_attr_groups, 
                                                            dev_data->lable
                                                         );
        if (IS_ERR(gpiodrv_data.dev[pos])) {
            dev_err(dev, "Create device error!\n");
            return PTR_ERR(gpiodrv_data.dev[pos]);
        }

        pos++;
    }
    return 0;
}

int gpio_sysfs_remove(struct platform_device *pdev) {
    
    int i;
    dev_info(&pdev->dev, "Remove call\n");

    for (i = 0; i < gpiodrv_data.total_device; i++) {
        device_unregister(gpiodrv_data.dev[i]);
    }
    return 0;
}

struct platform_driver gpio_platform_driver = {
    .probe = gpio_sysfs_probe,
    .remove = gpio_sysfs_remove,
    .driver = {
        .name = "bone-gpio-sysfs",
        .of_match_table = of_match_ptr(gpio_dt_match)
    }
};

static int __init gpio_sysfs_init(void) {

    int ret;

    /* Create class and device files </sys/class/...> */
    gpiodrv_data.class_gpio = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(gpiodrv_data.class_gpio)) {
        ret = PTR_ERR(gpiodrv_data.class_gpio);
        return ret;
    }

    ret = platform_driver_register(&gpio_platform_driver);
    if (ret < 0)
        goto class_del;

    pr_info("Platform driver module loaded\n");
    return 0;

class_del:
    class_destroy(gpiodrv_data.class_gpio);
    return ret;
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