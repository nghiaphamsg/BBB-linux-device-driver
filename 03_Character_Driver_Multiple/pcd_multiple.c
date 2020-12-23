/*
 * @brief: pseudo character device driver to support four pseudo character devices.
 *         Implement open/release/read/write/lseek driver methods to handle user requests.
 * @author: NghiaPham
 * @ver: v0.1
 * @date: 2020/09/26
 *
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#undef pr_fmt
#define pr_fmt(fmt) "[%s]: " fmt, __func__

#define CLASS_NAME      "pcd_class"
#define DEV_NAME        "pcdevs"
#define NO_OF_DEVICES   4

#define RDONLY          0x01
#define WRONLY          0x02
#define RDWR            0x03

#define MEM_SIZE_PCD1   1024
#define MEM_SIZE_PCD2   512
#define MEM_SIZE_PCD3   1024
#define MEM_SIZE_PCD4   512

char device_buffer_pcd1[MEM_SIZE_PCD1];
char device_buffer_pcd2[MEM_SIZE_PCD2];
char device_buffer_pcd3[MEM_SIZE_PCD3];
char device_buffer_pcd4[MEM_SIZE_PCD4];

/* Structure represents device private data */
struct pcdev_private_data {
    char *buffer;
    unsigned size;
    const char *serial_number;
    int permission;
    struct cdev cdev;
    struct mutex pcdev_lock;
};

/* Structure represents driver private data */
struct pcdrv_private_data {
    int total_device;
    struct pcdev_private_data pcdev_data[NO_OF_DEVICES];
    dev_t device_number;
    struct class *class_pcd;
    struct device *device_pcd;
};

/* Initialization every device */
struct pcdrv_private_data pcdrv_data = {
    .total_device = NO_OF_DEVICES,
    .pcdev_data = {
            [0] = {
                    .buffer = device_buffer_pcd1,
                    .size = MEM_SIZE_PCD1,
                    .serial_number = "PCD_DEV1",
                    .permission = RDONLY
            },
            [1] = {
                    .buffer = device_buffer_pcd2,
                    .size = MEM_SIZE_PCD2,
                    .serial_number = "PCD_DEV2",
                    .permission = WRONLY
            },
            [2] = {
                    .buffer = device_buffer_pcd3,
                    .size = MEM_SIZE_PCD3,
                    .serial_number = "PCD_DEV3",
                    .permission = RDWR
            },
            [3] = {
                    .buffer = device_buffer_pcd4,
                    .size = MEM_SIZE_PCD4,
                    .serial_number = "PCD_DEV4",
                    .permission = RDWR
            }
    }
};

/* The prototype functions for the character driver -- must come before the struct definition */
int check_permission(int permission, int access_mode);
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
    ret = check_permission(pcdev_data->permission, filp->f_mode);
    if (!ret)
        pr_info("Open was successful\n");
    else
        pr_info("Open was unsuccessful\n");

    return ret;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos) {

    int max_size;
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;
    max_size = pcdev_data->size;

    if (mutex_lock_interruptible(&pcdev_data->pcdev_lock))
        return -EINTR;

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

    mutex_unlock(&pcdev_data->pcdev_lock);
    return count;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos) {

    int max_size;
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;
    max_size = pcdev_data->size;

    if (mutex_lock_interruptible(&pcdev_data->pcdev_lock))
        return -EINTR;

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

    mutex_unlock(&pcdev_data->pcdev_lock);
    return count;
}

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence) {
    
    loff_t temp;
    unsigned int max_size;
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;
    max_size = pcdev_data->size;

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

static int __init char_device_driver_init(void) {

    int ret, i;

    /* Dynamically allocate a device number <one device> */
    ret = alloc_chrdev_region(&pcdrv_data.device_number, 0, NO_OF_DEVICES, DEV_NAME);
    if (ret < 0)
        goto error_out;

    /* Create class and device files </sys/class/...> */
    pcdrv_data.class_pcd = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(pcdrv_data.class_pcd)) {
        ret = PTR_ERR(pcdrv_data.class_pcd);
        goto unregister_char_dd;
    }

    for (i = 0; i < NO_OF_DEVICES; i++) {
        pr_info("Device number <Major>:<Minor> = %d:%d\n", MAJOR(pcdrv_data.device_number + i), \
                                                           MINOR(pcdrv_data.device_number + i));

        /* Initialize mutex for each device */
        mutex_init(&pcdrv_data.pcdev_data[i].pcdev_lock);

        /* Make a character device registration with the VFS */
        cdev_init(&pcdrv_data.pcdev_data[i].cdev, &pcd_fops);
        pcdrv_data.pcdev_data[i].cdev.owner = THIS_MODULE;
        ret = cdev_add(&pcdrv_data.pcdev_data[i].cdev, pcdrv_data.device_number + i, 1);
        if (ret < 0)
            goto cdev_del;

        pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, NULL, pcdrv_data.device_number + i, NULL, "pcdev-%d", i+1);
        if (IS_ERR(pcdrv_data.device_pcd)) {
            ret = PTR_ERR(pcdrv_data.device_pcd);
            goto class_del;
        }
    }

    pr_info("Module init was successful\n");

    return 0;

cdev_del:
class_del:
    for (; i >= 0; i--) {
        device_destroy(pcdrv_data.class_pcd, pcdrv_data.device_number + i);
        cdev_del(&pcdrv_data.pcdev_data[i].cdev);
    }
    class_destroy(pcdrv_data.class_pcd);
unregister_char_dd:
    unregister_chrdev_region(pcdrv_data.device_number, NO_OF_DEVICES);
error_out:
    return ret;

}

static void __exit char_device_driver_exit(void) {
    int i;

    for (i = 0; i < NO_OF_DEVICES; i++) {
        device_destroy(pcdrv_data.class_pcd, pcdrv_data.device_number + i);
        cdev_del(&pcdrv_data.pcdev_data[i].cdev);
    }
    class_destroy(pcdrv_data.class_pcd);
    unregister_chrdev_region(pcdrv_data.device_number, NO_OF_DEVICES);

    pr_info("Unload module\n");
}

module_init(char_device_driver_init);
module_exit(char_device_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NghiaPham");
MODULE_DESCRIPTION("Pseudo character device which handles multiple devices");
MODULE_INFO(board,"Beaglebone Black rev.c");