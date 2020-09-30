/*
 * @brief: this file was made to check the character multiple device 
 * @author: NghiaPham
 * @date: 2020/09/27
 * @version: v0.1
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define DEV_NAME    "/dev/pcdev-3"
#define BUFF_SIZE   2048

char tranfer_buff[BUFF_SIZE];
char receive_buff[BUFF_SIZE];

int user_write(void){
    int fd, ret;

    fd = open(DEV_NAME, O_WRONLY);
	if(fd < 0){
		perror("Failed to open the device\n");
        close(fd);
		return errno;
	}

    printf("Type in a short string to send to the kernel module:\n");
    scanf(" %[^\n]%*c", tranfer_buff);

    ret = write(fd, tranfer_buff, BUFF_SIZE);
    if (ret < 0) {
        printf("Failed to write the message to the device\n");
        close(fd);
        return errno;
    }

    close(fd);
    return 0;
}

int user_read(int size) {
    int fd, ret;
    int total_read = 0, tmp = 0;

    fd = open(DEV_NAME, O_RDONLY);
	if(fd < 0){
		perror("Failed to open the device\n");
        close(fd);
		return errno;
	}

    while(size) {
        tmp = read(fd, &receive_buff[total_read], size);

        if (!tmp) {
            printf("End of file \n");
            close(fd);
            break;
        } else if (tmp <= size) {
            printf("read %d bytes of data \n", tmp);
            /* 'ret' contains count of data bytes successfully read , so add it to 'total_read' */
            total_read += tmp;
            /* We read some data, so decrement 'remaining'*/
            size -= tmp;
        } else if (tmp < 0) {
            printf("something went wrong\n");
            close(fd);
            break;
        }
    }

    /* Dump buffer */
    for(int i = 0; i < total_read; i++)
        printf("%c",receive_buff[i]);

    printf("\n");
    close(fd);
    return 0;
}

int main(int argc, char *argv[])
{
    int size;

    if(argc < 2 || strcmp(argv[1], "--help") == 0){
        printf("Usage: %s [read/write] <read size> \n", argv[0]);
        printf("E.g. %s read 1024\n", argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "write") == 0)
        user_write();

    if (strcmp(argv[1], "read") == 0) {
        size = atoi(argv[2]);
        user_read(size);
    }

    /* Activate this for lseek testing */
#if  0 
	ret = lseek(fd,-10,SEEK_SET);
	if(ret < 0){
		perror("lseek");
		close(fd);
		return ret;
	}
#endif

    return 0;
}