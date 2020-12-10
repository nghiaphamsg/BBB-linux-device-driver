/*
 * @brief: Executable code for device creation, include device tree and device setup code.
 *         Implement sysfs & create custom attributes of a device.
 * @author: NghiaPham
 * @ver: v0.1
 * @date: 2020/12/05
 *
*/

#include "pcd_driver_dt_sysfs.h"

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

struct pcdrv_private_data pcdrv_data;

/* Create two custom attributes of a device. */
static DEVICE_ATTR(max_size, S_IRUGO | S_IWUSR, max_size_show, max_size_store);
static DEVICE_ATTR(serial_number, S_IRUGO, serial_number_show, NULL);

struct attribute *pcd_attrs[] = {
    &dev_attr_max_size.attr,
    &dev_attr_serial_number.attr,
    NULL
};

struct attribute_group pcd_attr_group = {
    .name = ATTR_GP_NAME,
    .attrs = pcd_attrs
};

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

/* Implement interface for exporting device attributes */
ssize_t max_size_show(struct device *dev, struct device_attribute *attr, char *buf) {

    struct pcdev_private_data *dev_data = dev_get_drvdata(dev->parent);
    return scnprintf(buf, PAGE_SIZE, "%d\n", dev_data->pdata.size);
}

ssize_t max_size_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {

    long result;
    int ret;
    struct pcdev_private_data *dev_data = dev_get_drvdata(dev->parent);
    
    ret = kstrtol(buf, 10, &result);
    if (ret)
        return ret;
    
    dev_data->pdata.size = result;
    dev_data->buffer = krealloc(dev_data->buffer, dev_data->pdata.size, GFP_KERNEL);
	return count;
}

ssize_t serial_number_show(struct device *dev, struct device_attribute *attr, char *buf) {

    struct pcdev_private_data *dev_data = dev_get_drvdata(dev->parent);
    return scnprintf(buf, PAGE_SIZE, "%s\n", dev_data->pdata.serial_number);
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

int pcd_sysfs_create(struct device *dev) {

    int ret;
    /* Create file sysfs in device class */
    ret = sysfs_create_file(&dev->kobj, &dev_attr_max_size.attr);
    if (ret)
        return ret;

    ret = sysfs_create_file(&dev->kobj, &dev_attr_serial_number.attr);
    if (ret)
        return ret;

    /* Create group sysfs in device class */
    ret = sysfs_create_group(&dev->kobj, &pcd_attr_group);
    if (ret)
        return ret;

    return 0;
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

    ret = pcd_sysfs_create(pcdrv_data.device_pcd);
    if (ret){
        device_destroy(pcdrv_data.class_pcd, dev_data->dev_num);
        return ret;
    }

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
MODULE_DESCRIPTION("System fs exercise");
MODULE_INFO(board,"Beaglebone Black rev.c");