<h1> Linux Device Driver Using BeagleBone Black </h1>

## 1. Introduce

- Device driver is a piece of code that configures and manages a device. 
- The device driver code knows, how to configure the device, sending data to the device, and it knows how to process requests which originate from the device. 
- When the device driver code is loaded into the operating system such as Linux, it exposes interfaces to the user-space so that the user application can communicate with the device. 
- Without the device driver, the OS/Application will not have a clear picture of how to deal with a device.
- There are three types of device drivers:
    + Character device drivers: character devices (RTC, keyboard, sensor,...)
    + Block device drivers: storage devices (mmc, eeproom, flash, harddisk,...)
    + Network device drivers: network devices (ethernet, wifi, bluetooth,...)

<p align="center"><img width="900" src="https://user-images.githubusercontent.com/32474027/94214748-5db8a700-ff15-11ea-8ee6-8e500a9f9f9a.PNG" \></p>

- **Kernel space**: This is a set of addresses where the kernel is hosted and where it runs. Kernel memory (or kernel space) is a memory range, owned by the kernel, protected by access flags, preventing any user apps from messing with the kernel (un)knowingly. On the other hand, the kernel can access the whole system memory, since it runs with the higher priority on the system. In kernel mode, the CPU can access the whole memory (both kernel space and user space).

- **User space**: This is a set of addresses (locations) where normal programs (such as `gedit` and so on) are restricted to run in. You may consider it a sandbox or a jail, so that a user program can't mess with memory or any other resource owned by another program. In user mode, the CPU can only access memory tagged with user space access rights. The only way for a user app to run in the kernel space is through system calls. Some of these are `read`, `write`, `open`, `close`, `mmap`, and so on. User space code runs with lower priority. When a process performs a system call, a software interrupt is sent to the kernel, which turns on privileged mode so that the process can run in kernel space. When the system call returns, the kernel turns off the privileged mode and the process is jailed again.

## 2. Prepare

| Software              | Hardware               |       
|-----------------------|------------------------|
| ubuntu 18.04/20.04    | Beaglebone Black Rev.C |
| cross compile 7 or 8  | cable, led, lcd        |
| vsc or vim            |


| Reference                                                                               |
|-----------------------------------------------------------------------------------------|
| [AM335x ARM® Cortex™-A8 Microprocessors (MPUs) Technical Reference Manual](https://e2e.ti.com/cfs-file/__key/communityserver-discussions-components-files/790/AM335x_5F00_techincal_5F00_reference_5F00_manual.pdf) |
| [AM335x Datasheet](https://www.ti.com/lit/ds/sprs717l/sprs717l.pdf?ts=1598362140689&ref_url=https%253A%252F%252Fwww.google.com%252F) |
| [Beagleboard Cape Expansion Headers](https://elinux.org/Beagleboard:Cape_Expansion_Headers)|
| [Device Tree Document](https://www.devicetree.org/specifications/) |

## 3. Source organization

- `arch/`: The Linux kernel is a fast growing project that supports more and more architectures. That being said, the kernel wants to be as generic as possible. Architecture-specific code is separated from the rest, and falls into this directory. This directory contains processor-specific subdirectories such as `alpha/`, `arm/`, `mips/`, `blackfin/`, and so on.
- `block/`: This directory contains code for block storage devices, actually the scheduling algorithm.
- `crypto/`: This directory contains the cryptographic API and the encryption algorithms code.
- `Documentation/`: This should be your favorite directory. It contains the descriptions of APIs used for different kernel frameworks and subsystems. You should look here prior to asking any questions on forums.
- `drivers/`: This is the heaviest directory, continuously growing as device drivers get merged. It contains every device driver organized in various subdirectories.
- `fs/`: This directory contains the implementation of different filesystems that the kernel actually supports, such as *NTFS*, *FAT*, *ETX{2,3,4}*, *sysfs*, *procfs*, *NFS*, and so on.
- `include/`: This contains kernel header files.
- `init/`: This directory contains the initialization and start up code.
- `ipc/`: This contains implementation of the **Inter-Process Communication (IPC)** mechanisms, such as *message queues*, *semaphores*, and *hared memory*...
- `kernel/`: This directory contains architecture-independent portions of the base kernel.
- `lib/`: Library routines and some helper functions live here. They are generic **kernel object (kobject)** handlers, **Cyclic Redundancy Code (CRC)** computation functions, and so on.
- `mm/`: This contains memory management code.
- `net/`: This contains networking (whatever network type it is) protocols code.
- `scripts/`: This contains scripts and tools used during kernel development. There are other useful tools here.
- `security/`: This directory contains the security framework code.
- `sound/`: Audio subsystems code is here.
- `usr/`: This currently contains the initramfs implementation.

## 4. Kernel configuration
- In most cases, there will be no need to start a configuration from scratch. There are default and useful configuration files available in each `arch/` directory, which you can use as a starting point:
```shell
ls arch/<you_arch>/configs/ 
```
- For ARM-based CPUs, these configs files are located in `arch/arm/configs/`, and for an BeagleBone Black processor, the default file config is `arch/arm/configs/bb.org_defconfig`. Similarly, for x86 processors we find the files in `arch/x86/configs/`, with only two default configuration files, `i386_defconfig` and `x86_64_defconfig`, for 32-bit and 64-bit versions respectively.
- For an x86 system:
```shell
make x86_64_defconfig
make uImage -j4
make modules
make INSTALL_MOD_PATH=</where/to/install> modules_install
```
- For BBB board:
```shell
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- bb.org_defconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- uImage dtbs LOADADDR=0x80008000 -j4
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules -j4
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- INSTALL_MOD_PATH=</where/to/install> modules_install
```
















