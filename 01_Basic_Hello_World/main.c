/* 
 * @brief: basic linux kernel device driver
 * @author: NghiaPham
 * @ver: v0.1
 * 
*/

#include <linux/module.h>

static int __init helloworld_init(void) {
    printk("Hello World\n");
    return 0;
}

static void __exit helloworld_exit(void) {
    printk("Bye World\n");
}

module_init(helloworld_init);
module_exit(helloworld_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NghiaPham");
MODULE_DESCRIPTION("Simple hello world kernel module");
MODULE_INFO(board,"Beaglebone black rev.c");