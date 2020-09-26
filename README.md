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

## 2. Prepare

| Sortware          | Hardware              |       
|-------------------|-----------------------|
| Ubuntu 18.04      | Beaglebone Black Rev.C|
| Cross Compile     |
| VSC or Vim        |


| Reference                                                                               |
|-----------------------------------------------------------------------------------------|
| [AM335x ARM® Cortex™-A8 Microprocessors (MPUs) Technical Reference Manual](https://e2e.ti.com/cfs-file/__key/communityserver-discussions-components-files/790/AM335x_5F00_techincal_5F00_reference_5F00_manual.pdf) |
| [AM335x Datasheet](https://www.ti.com/lit/ds/sprs717l/sprs717l.pdf?ts=1598362140689&ref_url=https%253A%252F%252Fwww.google.com%252F) |
| [Beagleboard Cape Expansion Headers](https://elinux.org/Beagleboard:Cape_Expansion_Headers)|
