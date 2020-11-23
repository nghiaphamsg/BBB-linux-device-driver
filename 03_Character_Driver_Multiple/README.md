<h1> Character Device Driver Multiple </h1>

## 1. Code exercise
- Modify the previous pseudo character device driver to support four pseudo character devices.
- Implement open, release, read, write, lseek driver methods to handle user requests.

<p align="center"> <img width="750" src="https://user-images.githubusercontent.com/32474027/94355859-bed1ad80-00c2-11eb-9881-52bdc862f264.png" \></p>

- The driver, which we about to write, should manage 4 devices.
- There will be single driver managing 4 devices.
- There will be only 1 implementation of open, read, write, release, and lseek driver methods.
- That also means the driver should first determine which device is being accessed from the user space to fulfill the request

## 2. Explore about device/driver data structure

- In the driver, we’ll maintain two structures:
    + Structure which holds driver’s private data
    + Structure which holds device’s private data

```text
struct pcdrv_private_data {
    int total_device;
    struct pcdev_private_data pcdev_data[NO_OF_DEVICES];
}
```
<p align="center"> <img width="750" src="https://user-images.githubusercontent.com/32474027/94355919-79fa4680-00c3-11eb-8a85-6a9a958ae4e3.png"\> </p>

```text
struct pcdev_private_data {
    char *buffer;
    unsigned size;
    const char *serial_number;
    int permission;
    struct cdev cdev;
}
```

## 3. Testing
- Step 1: Generate `*.ko` and test pcd object
```shell
root@nekobot:~/03_Character_Driver_Multiple# make
```
- Step 2: Load into kernel module
```shell
root@nekobot:~/03_Character_Driver_Multiple# insmod pcd_multiple.ko
```
- Step 3: Using pcd_mul_test for write
```shell
$ root@nekobot:~/03_Character_Driver_Multiple# ./pcd_mul_test write
    Type in a short string to send to the kernel module:
    You are my apple
```
- Step 4: Using pcd_mul_test for read
```shell
$ root@nekobot:~/03_Character_Driver_Multiple# ./pcd_mul_test read 1024
    You are my apple
```
- Step 4: Unload module
```shell
root@nekobot:~/03_Character_Driver_Multiple# rmmod pcd_multiple
```
**Note:** You can check kernel log by command `dmesg | tail`


















