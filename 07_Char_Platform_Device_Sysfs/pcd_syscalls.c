/*
 * @brief: General system calls for userspace to use
 * @author: NghiaPham
 * @ver: v0.1
 * @date: 2020/12/05
 *
*/

#include "pcd_driver_dt_sysfs.h"

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