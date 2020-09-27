/*
 * @brief: this file was made to check the character device
 * @author: NghiaPham
 * @date: 2020/09/26
 * @version: v0.1
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define DEV_PATH    "/dev/pcd"
#define BUFF_SIZE   512

char stringSend[BUFF_SIZE];
char receive[BUFF_SIZE];

int main() {
    
    int fd, ret;
    char option;

    printf("Starting device test code example...\n");
    fd = open(DEV_PATH, O_RDWR);
    if (fd < 0) {
        printf("Failed to open the device\n");
        return errno;
    }

    printf("Type in a short string to send to the kernel module:\n");
    scanf(" %[^\n]%*c", stringSend);

    ret = write(fd, stringSend, BUFF_SIZE);
    if (ret < 0) {
        printf("Failed to write the message to the device\n");
        return errno;
    }

    printf("Press ENTER to read back from the device...\n");
    getchar();

    ret = read(fd, receive, BUFF_SIZE);
    if (ret < 0) {
        printf("Failed to read the message from the device\n");
        return errno;
    }
    printf("The received message is: %s\n\n", receive);

    printf("End of the program\n");
    return 0;
}
