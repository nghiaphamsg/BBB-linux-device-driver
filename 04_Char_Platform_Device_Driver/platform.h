#define RDONLY      0x01
#define WRONLY      0x02
#define RDWR        0x03

struct pcdev_platform_data {
    int size;
    int permission;
    const char *serial_number;
};