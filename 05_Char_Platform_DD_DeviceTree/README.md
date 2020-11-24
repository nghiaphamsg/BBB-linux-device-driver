<h1> Exploring Device Tree </h1>

## 1. Code exercise
- Repeat the exercise pseudo character driver with multiple devices as a platform driver.
- The driver must give open, release, read, write, lseek methods to deal with the devices
- Create **device tree** to represent platform devices

## 2. What is device tree ?
### 2.1 Concept
- The “Open Firmware Device Tree”, or simply Device Tree (DT), is a data structure and language for describing hardware. More specifically, it is a description of hardware that is readable by an operating system so that the operating system doesn’t need to hard code details of the machine.
- Structurally, the DT is a tree, or acyclic graph with named nodes, and nodes may have an arbitrary number of named properties encapsulating arbitrary data. A mechanism also exists to create arbitrary links from one node to another outside of the natural tree structure.
- Conceptually, a common set of usage conventions, called ‘bindings’, is defined for how data should appear in the tree to describe typical hardware characteristics including data busses, interrupt lines, GPIO connections, and peripheral devices.

- In short, it is a new and recommended way to describe non-discoverable devices(platform devices) to the Linux kernel, which was previously hardcoded into kernel source files. 
**Source:** *Documentation/devicetree/usage-model.txt*

### 2.2 Why DT is used ?
Linux uses DT for:
- Platform identification
- Device population:
    + The kernel parses the device tree data and generates the required software data structure, which will be used by the kernel code.
- Ideally, the device tree is independent of any OS; when you change the OS, you can still use the same device tree file to describe the hardware to the new OS. That is the device tree makes “adding of device information“ independent of OS.

Writing device tree: *https://elinux.org/Device_Tree_Usage*

### 2.3 Device tree bindings
- Device tree binding document. The driver writer must document these details
- The properties that are necessary to describe a device in the device tree depends on the requirements of the Linux driver for that device
- When all the required properties are provided, the driver of charge can detect the device from the device tree and configure it
- Compatible strings and properties are first defined by the client program (OS, drivers) then shared with DT writer
![Screenshot from 2020-11-24 22-06-54](https://user-images.githubusercontent.com/32474027/100098450-69870200-2ea1-11eb-8c0c-63d9f51564b6.png)

- `of_match_device`-Tell if a struct device matches an `of_device_id` list, Used by a driver to check whether an `platform_device` present in the system is in its list of supported devices.

```
const struct of_device_id *of_match_device(const struct of_device_id *matches, const struct device *dev)
or
const void *of_device_get_match_data(const struct device *dev)
```

- All device tree binding document can be found here:
*https://www.kernel.org/doc/Documentation/devicetree/bindings/*

### 3. Structure device
- In Linux every device is represented by an instance of struct device
- Example node:
    + The I2C controller is located at offset `0x3000` from it's parent
    + The driver for the I2C controller is `fsl-i2c`
    + The first child is named `dtt` at offset `0x48` from it's parent and the driver is `national,lm75`
    + The second child is named `rtc` at offset `0x68` from it's parent and the driver is `dallas,ds1337`
![Screenshot from 2020-11-24 22-16-07](https://user-images.githubusercontent.com/32474027/100099379-b4554980-2ea2-11eb-8b54-d7da808d079e.png)

- And my device tree nodes for pcd driver:
![Screenshot from 2020-11-24 22-13-08](https://user-images.githubusercontent.com/32474027/100099206-735d3500-2ea2-11eb-937c-d4468dc3056c.png)

### 4. Create a new device tree
**Note:** Linux source code must build before. I used [linux version 5.4.70](https://github.com/beagleboard/linux) ([Build Tutorial](https://github.com/nghiaphamsg/BeagleBone_Black_Embedded/tree/master/02_Gerenate_UBoot_RFS#-the-third-stage-generate-linux-image-))

- Step 1: Create a new device tree (.dtsi)
```shell
cd /PATH_TO_LINUX_SOURCE/arch/arm/boot/dts
vi am335x_boneblack_ldd.dtsi
```

- Step 2: Include .dtsi into main .dts file (#include "am335x_boneblack_ldd.dtsi")
```shell
vi /PATH_TO_LINUX_SOURCE/arch/arm/boot/dts/am335x-boneblack.dts
```

- Step 3: Rebuild kernel
```
cd /PATH_TO_LINUX_SOURCE
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- am335x-boneblack.dtb
```
![Screenshot from 2020-11-24 23-07-06](https://user-images.githubusercontent.com/32474027/100104972-2aa97a00-2eaa-11eb-9dd8-45eaf8e858c8.png)

Step 4: Update device tree binary in SD card
```shell
cp arch/arm/boot/dts/am335x-boneblack.dtb /media/YOURNAME/BOOT/
```

Step 5: Check platform device (target machine)

![Screenshot from 2020-11-24 23-14-22](https://user-images.githubusercontent.com/32474027/100105634-f1253e80-2eaa-11eb-9505-2097d978c776.png)

### 5. Test pcdev driver
- Step 1: Make kernel object (target machine)
```shell
make
```
- Step 2: Copy driver from machine to target
```
scp pcd_driver_dt.ko debian@192.168.7.2:/home/debian/
```

- Step 3: Load driver module
```
sudo insmod pcd_driver_dt.ko
```
![Screenshot from 2020-11-24 23-22-40](https://user-images.githubusercontent.com/32474027/100106556-0484d980-2eac-11eb-9434-44f12136e71d.png)

- Step 4: Unload module
```
sudo rmmod pcd_driver_dt
```
![Screenshot from 2020-11-24 23-25-00](https://user-images.githubusercontent.com/32474027/100106773-4e6dbf80-2eac-11eb-8599-8f5d688be8cc.png)