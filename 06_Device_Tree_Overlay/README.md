<h1> Device Tree Overlays </h1>

## 1. Exercise
- Write a device tree overlay to disable/modify pcdev device nodes from the main dts

**Note:** Reusing source code [05_Char_Platform_DD_DeviceTree](https://github.com/nghiaphamsg/BeagleBoneBlack_Linux_Device_Driver/tree/master/05_Char_Platform_DD_DeviceTree).

## 2. Discuss about Device Tree Overlays (DTO)
- DT overlays are device tree patches(dtbo) which are used to patch or modify the existing main device tree blob(dtb)
- Uses of overlays:
    + To support and manage hardware configuration (properties, nodes, pin configurations) of various capes of the board
    + To alter the properties of already existing device nodes of the main dtb
    + Overlays approach maintains modularity and makes capes management easier

- Overlay DTS Format:
```
    The DTS of an overlay should have the following format:

    {
        /* ignored properties by the overlay */

        fragment@0 {	/* first child node */

            target=<phandle>;	/* phandle target of the overlay */
        or
            target-path="/path";	/* target path of the overlay */

            __overlay__ {
                property-a;	/* add property-a to the target */
                node-a {	/* add to an existing, or create a node-a */
                    ...
                };
            };
        }
        fragment@1 {	/* second child node */
            ...
        };
        /* more fragments follow */
    }
```
- Example:
![Screenshot from 2020-11-24 23-53-20](https://user-images.githubusercontent.com/32474027/100110499-59c2ea00-2eb0-11eb-848b-b5fa2020fe39.png)

- References: *https://www.kernel.org/doc/Documentation/devicetree/overlay-notes.txt*

## 3. Steps create DTOs and compilation
- Step 1: Create a device tree overlay file and add fragments to modify the target device nodes
- Step 2: Compile the device tree overlay file to generate `.dtbo` file (*Make sure that device tree compiler(dtc) is installed on your system*)
```shell
sudo apt-get install device-tree-compiler
dtc -@ -I dts -O dtb -o pcdev_dto.dto example1.dts
```
![Screenshot from 2020-11-25 00-12-34](https://user-images.githubusercontent.com/32474027/100112792-fd14fe80-2eb2-11eb-9155-0757842d670c.png)

- Step 3: Copy `.dto` to target & save in `/lib/firmware`
```
sudo scp pcdev_dto.dto debian@192.168.7.2:/home/debian/
sudo cp pcdev_dto.dto /lib/firmware/
```

## 4. Load DTO from u-boot command
Documment: */UBOOT_PATH/doc/README.fdt-overlays*

Step 1: Reboot board & jump in u-boot console (press any key)
Step 2: Set environment 
```shell
setenv fdtaddr 0x87f00000
setenv fdtovaddr 0x87fc0000
setenv bootargs console=ttyO0,115200n8 root=/dev/mmcblk0p2 rw rootfstype=ext4 rootwait earlyprintk mem=512M
```

Step 3: Load the uImage, base blob and overlay blobs
```shell
//load ${devtype} ${bootpart} ${fdtaddr} ${bootdir}/base.dtb
load mmc 0:1 ${fdtaddr} am335x-boneblack.dtb

//load ${devtype} ${bootpart} ${fdtovaddr} ${rootdir}/overlay.dtb
load mmc 0:2 ${fdtovaddr} /lib/firmware/pcdev_dto.dto

//load ${devtype} ${bootpart} ${loadaddr} ${bootdir}/uImage
load mmc 0:1 ${loadaddr} uImage
```

Step 4: Set it as the working fdt tree
```shell
fdt addr $fdtaddr
```

Step 5: Grow it enough so it can 'fit' all the applied overlays
```shell
fdt resize 8192
```

Step 6: You are now ready to apply the overlay
```shell
fdt apply $fdtovaddr
```

Step 7: Boot system like you would do with a traditional dtb
```shell
bootm ${loadaddr} - ${fdtaddr}
```
![Screenshot from 2020-11-25 22-25-26](https://user-images.githubusercontent.com/32474027/100233980-c8b24880-2f6d-11eb-8829-f9fc34340e62.png)

Step 8: Check pcdev again. This time only shows 2 devices pcdev3 & pcdev4 (pcdev1 & pcdev2 have been set status disabled)

![Screenshot from 2020-11-25 22-29-21](https://user-images.githubusercontent.com/32474027/100234298-39596500-2f6e-11eb-9c48-d93907021e2e.png)

## 5. Load DTO from uEnv.txt
- Using a new uEnv.txt file