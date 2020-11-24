<h1> Device Tree Overlays </h1>

## 1. Code example
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
- Step 2: Compile the device tree overlay file to generate `.dtbo` file (Device tree overlay binary)
*Make sure that device tree compiler(dtc) is installed on your system*
```shell
sudo apt-get install device-tree-compiler
dtc -@ -I dts -O dtb -o pcdev_dto.dto example1.dts
```
![Screenshot from 2020-11-25 00-12-34](https://user-images.githubusercontent.com/32474027/100112792-fd14fe80-2eb2-11eb-9155-0757842d670c.png)

- Step 3: Copy `.dto` to target
```
sudo scp pcdev_dto.dto debian@192.168.7.2:/home/debian/
sudo cp pcdev_dto.dto /lib/firmware/
```

- Step 4: Make u-boot to load the `.dtbo` file during board start-up
