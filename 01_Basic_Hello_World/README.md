<h1> Linux Kernel Module (LKM) </h1>

- Linux supports dynamic insertion and removal of code from the kernel while the system is up and running. The code what we add and remove at run time is called a kernel module.
- Once the LKM is loaded into the Linux kernel, you can start using new features and functionalities exposed by the kernel module without even restarting the device.
- LKM dynamically extends the functionality of the kernel by introducing new features to the kernel such as security, device drivers, file system drivers, system
calls etc. (modular approach)
- Example: when a hot-pluggable new device is inserted the device driver which is an LKM gets loaded automatically to the kernel.

## 1. Static and dynamic LKMs
- **Static** (y): When you build a Linux kernel, you can make your module statically linked to the kernel image (module becomes part of the final Linux kernel image). This method increases the size of the final Linux kernel image. Since the module is **built-in** into the Linux kernel image, you can **NOT unload** the module. It occupies the memory permanently during run time.

- **Dynamic** (m): When you build a Linux kernel, these modules are **NOT built** into the final kernel image, and rather there are compiled and linked separately to produce `.ko` files. You can dynamically **load** and **unload** these modules from the kernel using user space programs such as `insmod`, `modprobe`, `rmmod`.


## 2. Building a kernel module
- Two ways:
  + Statically linked against the kernel image
  + Dynamically loadable
  
### 2.1 Dynamically loadable

<p align="center"><img width="850" src="https://user-images.githubusercontent.com/32474027/93759770-cf5ad180-fc45-11ea-9b71-69de02fb0b01.png"/></p>

- The modules which are already part of the Linux Kernel are called **in-tree modules**. (approved by the kernel developers and maintainers)
- When you write a module separately (which is not approved and may be buggy), build and link it against the running kernel, then its called as **out of the tree module**.
- Modules are built using **"kbuild"** which is the build system used by the Linux kernel.
- Modules must use **"kbuild"** to stay compatible with changes in the build infrastructure and to pick up the right flags to GCC.
- To build external modules, **you must have a prebuilt kernel source available** that contains the configuration and header files used in the build. ([The Third Stage: Generate Linux Image](https://github.com/nghiaphamsg/BeagleBone_Black_Embedded/tree/master/02_Gerenate_UBoot_RFS))

### 2.1.1 Out of tree module

#### Commad syntax
```shell
make -C $KERNEL_DIR M=$SOURCE_DIR [Options]
```
  + `-C $KERNEL_DIR`: The directory where the kernel source is located. **"make"** will actually change to the specified directory when executing and will change back when finished
  + `M=$SOURCE_DIR`: Informs kbuild that an external module is being built. The value given to **"M"** is the absolute path of the directory where the external module (kbuild file) is located.
  + `Option`: \
              + **modules** :The default target for external modules. It has the same functionality as if no target was specified.\
              + **modules_install**: Install the external module(s). The default location is `/lib/modules/<kernel_release>/extra/`.\
              + **clean**: Remove all generated files in the module directory only.\
              + **help**:List the available targets for external modules.

#### Creating a host Makefile
```shell
obj-m := main.o

ARCH=arm
CC=arm-linux-gnueabihf-
KERNEL_DIR=/<prebuilt_kernel_source_dir>/

all:
  make ARCH=$(ARCH) CROSS_COMIPLE=$(CC) -C $(KERNEL_DIR) M=$(shell pwd) modules
clean:
  make ARCH=$(ARCH) CROSS_COMIPLE=$(CC) -C $(KERNEL_DIR) M=$(shell pwd) clean
help:
  make ARCH=$(ARCH) CROSS_COMIPLE=$(CC) -C $(KERNEL_DIR) M=$(shell pwd) help
```


#### Creating a target Makefile
- The first thing to do is to install Linux kernel header files that perfectly align with the Linux kernel distribution on your device or machine.
- The `uname` command provides a long description (-a for all) and a kernel release output (-r for release) as follows:
```shell
root@beaglebone:~# uname -a
Linux beaglebone 4.14.108-ti-r113 #1 SMP PREEMPT Wed Jul 31 00:01:10 UTC 2019 armv7l GNU/Linux
```
- The kernel release output can be used to search for the appropriate Linux header files
```shell
root@beaglebone:~# apt-cache search linux-headers-$(uname -r)
linux-headers-4.14.108-ti-r113 - Linux kernel headers for 4.14.108-ti-r113 on armhf
root@beaglebone:~# apt install linux-headers-$(uname -r)
```
- The headers should be installed in `/lib/modules/$(uname -r)/build/`, which should likely be a symbolic link to the location `/usr/src/linux/$(uname -r)/`.
```shell
root@beaglebone:/# cd /usr/src; ls
linux-headers-4.14.108-ti-r113

root@beaglebone:/lib/modules/4.14.108-ti-r113# ls -l build
lrwxrwxrwx 1 root root 39 Jul 31  2019 build -> /usr/src/linux-headers-4.14.108-ti-r113
```
- Creating a makefile
```shell
obj-m := main.o

KERNEL_DIR=/lib/modules/$(shell uname -r)/build/

all:
  make -C $(KERNEL_DIR) M=$(shell pwd) modules
clean:
  make -C $(KERNEL_DIR) M=$(shell pwd) clean
help:
  make -C $(KERNEL_DIR) M=$(shell pwd) help
```
### 2.1.2 In tree module (host)
- Steps to add in-tree module to kernel menu configuration (character)

```text
my_char
  ├── Kconfig
  ├── main.c
  └── Makefile
```
Step 1: Create a new folder in `/driver/char/<folder_name>/` and copy source c into here
```shell
neko@nekobot:~/linux_4_14/drivers/char$ mkdir my_char; cd my_char
neko@nekobot:~/linux_4_14/drivers/char/my_char$ cp /01_Basic_Hello_World/main.c ./
```
Step 2: Create a `Kconfig` file and add below entries
```shell
neko@nekobot:~/linux_4_14/drivers/char/my_char$ vi Kconfig
```
```text
menu "My custom module"
	config CUSTOM_HELLOWORD
		tristate "Hello world module"
		default n
endmenu
```
Step 3: Create a `Makefile` file and add below entries
```shell
neko@nekobot:~/linux_4_14/drivers/char/my_char$ vi Makefile
```
```text
obj-$(CONFIG_CUSTOM_HELLOWORD) += main.o
```
Step 4: Add the local Kconfig entry to upper level Kconfig
```shell
neko@nekobot:~/linux_4_14/drivers/char$ vi Kconfig
```
```text
source "drivers/char/my_char/Kconfig"
```
Step 5: add the local level Makefile to higher level Makefile
```shell
neko@nekobot:~/linux_4_14/drivers/char$ vi Makefile
```
```text
obj-y				+= my_char
```

## 3. Testing the first LKM
### 3.1 Dynamically loadable
### 3.1.1 Out of tree module

Step 1: use `make` command
```shell
debian@beaglebone:~/ldd$ sudo make
make -C /lib/modules/4.14.108-ti-r113/build/ M=/home/debian/ldd modules
make[1]: Entering directory '/usr/src/linux-headers-4.14.108-ti-r113'
  CC [M]  /home/debian/ldd/main.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/debian/ldd/main.mod.o
  LD [M]  /home/debian/ldd/main.ko
make[1]: Leaving directory '/usr/src/linux-headers-4.14.108-ti-r113'

debian@beaglebone:~/ldd$ ls
main.c	main.ko  main.mod.c  main.mod.o  main.o  Makefile  modules.order  Module.symvers
```

Step 2: use `insmod` command for load LKM
```shell
debian@beaglebone:~/ldd$ sudo insmod main.ko

debian@beaglebone:~/ldd$ dmesg | tail
[   33.801773] configfs-gadget gadget: high-speed config #1: c
[   34.237082] IPv6: ADDRCONF(NETDEV_UP): usb0: link is not ready
[   34.303546] IPv6: ADDRCONF(NETDEV_CHANGE): usb0: link becomes ready
[  483.279437] main: disagrees about version of symbol module_layout
[ 1069.960646] main: loading out-of-tree module taints kernel.
[ 1069.970130] Hello World
```
Step 3: use `rmmod` command for unload LKM
```shell
debian@beaglebone:~/ldd$ sudo rmmod main.ko

debian@beaglebone:~/ldd$ dmesg | tail
[   33.801773] configfs-gadget gadget: high-speed config #1: c
[   34.237082] IPv6: ADDRCONF(NETDEV_UP): usb0: link is not ready
[   34.303546] IPv6: ADDRCONF(NETDEV_CHANGE): usb0: link becomes ready
[  483.279437] main: disagrees about version of symbol module_layout
[ 1069.960646] main: loading out-of-tree module taints kernel.
[ 1069.970130] Hello World
[ 1090.128383] Bye World
```
Step 4: use `make clean`
```shell
debian@beaglebone:~/ldd$ sudo make clean
make -C /lib/modules/4.14.108-ti-r113/build/ M=/home/debian/ldd clean
make[1]: Entering directory '/usr/src/linux-headers-4.14.108-ti-r113'
  CLEAN   /home/debian/ldd/.tmp_versions
  CLEAN   /home/debian/ldd/Module.symvers
make[1]: Leaving directory '/usr/src/linux-headers-4.14.108-ti-r113'
debian@beaglebone:~/ldd$ ls
main.c	Makefile
```
### 3.1.2 In tree module ([The Third Stage: Generate Linux Image](https://github.com/nghiaphamsg/BeagleBone_Black_Embedded/tree/master/02_Gerenate_UBoot_RFS))
- Build again linux kernel modules

Step 1: Open menu configure
```shell
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig
```
Step 2: Configuration a "Hello world" module by following
<p align="center"> <img width="800" src="https://user-images.githubusercontent.com/32474027/93771456-7cd6e080-fc58-11ea-9001-3565082032c6.png"/> </p>

Step 3: Compile and generate dynamic loadable kernel modules with the extension `.ko`
```shell
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules -j4
```

<p align="center"> <img width="1000" src="https://user-images.githubusercontent.com/32474027/93774939-c75a5c00-fc5c-11ea-85e9-34a1eee5ff52.png"/> </p>

Step 4: Check `.ko` file generated and look at `intree: Y`
```shell
neko@nekobot:~/linux_4_14/drivers/char/my_char$ ls
Kconfig  main.c  main.ko  main.mod.c  main.mod.o  main.o  Makefile  modules.builtin  modules.order

neko@nekobot:~/linux_4_14/drivers/char/my_char$ modinfo main.ko
filename:       /home/neko/Project/BeagleBoneBlack_Linux_Device_Driver/linux_4_14/drivers/char/my_char/main.ko
board:          Beaglebone black rev.c
description:    Simple hello world kernel module
author:         NghiaPham
license:        GPL
depends:        
intree:         Y
name:           main
vermagic:       4.14.108+ SMP preempt mod_unload modversions ARMv7 p2v8
```
