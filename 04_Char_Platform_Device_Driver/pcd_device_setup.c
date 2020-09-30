/*
 * @brief: Create 2 platform devices and initialize them with required information
 *         Register platform devices with the Linux kernel
 * @author: NghiaPham
 * @date: 2020/09/30
 * @version: v0.1
 * 
 */
#include <linux/module.h>
#include <linux/platform_device.h>
#include "platform.h"

#undef pr_fmt
#define pr_fmt(fmt) "[%s]: " fmt, __func__

/* Callback to free the device after all references have gone away */
void pcdev_release(struct device *dev) {
    // TODO: 
    pr_info("Device released\n");
}

/* Create two platform data */
struct pcdev_platform_data pcdev_pdata[2] = {
    [0] = {.size = 512, .permission = RDWR, .serial_number = "PCDEV_1"},
    [1] = {.size = 512, .permission = RDWR, .serial_number = "PCDEV_2"}
};

/* Create two platform device */
struct platform_device platform_pcdev_1 = {
    .name = "pseudo-char-device",
    .id = 0,
    .dev = {
        .platform_data = &pcdev_pdata[0],
        .release = pcdev_release
    }
};

struct platform_device platform_pcdev_2 = {
    .name = "pseudo-char-device",
    .id = 1,
    .dev = {
        .platform_data = &pcdev_pdata[1],
        .release = pcdev_release
    }
};

static int __init pcdev_platform_init(void) {
    /* Register platform-level device */
    platform_device_register(&platform_pcdev_1);
    platform_device_register(&platform_pcdev_2);

    pr_info("Platform device setup module loaded\n");

    return 0;
}

static void __exit pcdev_platform_exit(void) {
    /* Unregister platform-level device */
    platform_device_unregister(&platform_pcdev_1);
    platform_device_unregister(&platform_pcdev_2);

    pr_info("Platform device setup module unloaded\n");
}

module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NghiaPham");
MODULE_DESCRIPTION("Register platform devices with the Linux kernel");
MODULE_INFO(board,"Beaglebone Black rev.c");