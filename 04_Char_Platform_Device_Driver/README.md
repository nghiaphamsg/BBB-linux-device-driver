<h1> Exploring Platform Device and Platform Driver </h1>

## 1. Code exercise
- Repeat the exercise pseudo character driver with multiple devices as a platform driver.
- The driver should support multiple pseudo character devices(pcdevs) as platform devices
- Create device files to represent platform devices
- The driver must give open, release, read, write, lseek methods to deal with the devices

## 2. Discuss about platform device and platform driver
### 2.1 Platform device
- Platform devices are devices that typically appear as autonomous entities in the system. This includes legacy port-based devices and host bridges to peripheral buses, and most controllers integrated into system-on-chip platforms. What they usually have in common is direct addressing from a CPU bus. Rarely, a platform_device will be connected through a segment of some other kind of bus; but its registers will still be directly addressable.
- Platform devices are given a `name`, used in driver binding, and a list of resources such as addresses and IRQs:

```
struct platform_device {
      const char      *name;
      u32             id;
      struct device   dev;
      u32             num_resources;
      struct resource *resource;
};
```
- As a rule, **platform specific (and often board-specific) setup code** will register platform devices:
```
int platform_device_register(struct platform_device *pdev);
int platform_add_devices(struct platform_device **pdevs, int ndev);
```

### 2.2 Platform driver

- Platform drivers follow the standard driver model convention, where discovery/enumeration is handled outside the drivers, and drivers provide `probe()` and `remove()` methods. They support power management and shutdown notifications using the standard conventions:

```
struct platform_driver {
      int (*probe)(struct platform_device *);
      int (*remove)(struct platform_device *);
      void (*shutdown)(struct platform_device *);
      int (*suspend)(struct platform_device *, pm_message_t state);
      int (*suspend_late)(struct platform_device *, pm_message_t state);
      int (*resume_early)(struct platform_device *);
      int (*resume)(struct platform_device *);
      struct device_driver driver;
};
```

- Platform drivers register themselves the normal way:
```
int platform_driver_register(struct platform_driver *drv);
```
See more here: [Platform Devices and Drivers](https://www.kernel.org/doc/html/latest/driver-api/driver-model/platform.html)

## 3. Platform Device – Driver matching (binding)
- It’s built from two components:
    + `platform_device.name`; which is also used to for driver matching.
    + `platform_device.id`: the device instance number, or else “-1” to indicate there’s only one.

- `platform_match` - bind platform device to platform driver (*/drivers/base/platform.c*)
```
struct bus_type platform_bus_type = {
	.name		= "platform",
	.dev_groups	= platform_dev_groups,
	.match		= platform_match,
	.uevent		= platform_uevent,
	.dma_configure	= platform_dma_configure,
	.pm		= &platform_dev_pm_ops,
};

- The Linux platform core implementation maintains platform device and driver lists. Whenever you add a new platform device or driver, this list gets updated and matching mechanism triggers.

![Screenshot from 2020-11-23 17-13-15](https://user-images.githubusercontent.com/32474027/99940321-454af880-2daf-11eb-9f38-80eee04cb6ff.png)

- Add new platform device case:

![Screenshot from 2020-11-23 17-15-03](https://user-images.githubusercontent.com/32474027/99940424-79beb480-2daf-11eb-8469-418bc4be395b.png)

- Add new platform driver case:

![Screenshot from 2020-11-23 17-16-00](https://user-images.githubusercontent.com/32474027/99941404-51d05080-2db1-11eb-87bb-53d3bdef734e.png)

- Whenever a new device or a new driver is added, binding is performed automatically by the driver core, and if it finds a matching platform device for a platform driver, the `probe()` function of the matched driver will get called. Inside the `probe()` function, the driver configures the detected device. 
- Details of the matched platform device will be passed to the `probe()` function of the matched driver so that driver can extract the platform data and configure it.

## 4. Testing
- Step 1: Build kernel object (host machine)
```shell
sudo make
```
![step1](https://user-images.githubusercontent.com/32474027/99943026-f94e8280-2db3-11eb-8447-eeeb434cbf97.png)

- Step 2: Load platform device & show log
```shell
sudo insmod pcd_device_setup.ko
dmesg | tail -1
```
![step2](https://user-images.githubusercontent.com/32474027/99943108-1f742280-2db4-11eb-9274-5e1bea17f669.png)

- Step 3: Load platform driver & show log
```shell
sudo insmod pcd_driver.ko
dmesg | tail
```
![step3](https://user-images.githubusercontent.com/32474027/99943215-47638600-2db4-11eb-81ad-5a846ac9f0df.png)

- Step 4: Check platform driver & device

![Screenshot from 2020-11-23 17-55-42](https://user-images.githubusercontent.com/32474027/99943846-4ed75f00-2db5-11eb-91a3-265eb2112403.png)

- Step 5: Unload modules
```shell
sudo rmmod pcd_device_setup
sudo rmmod pcd_driver
dmesg tail
```