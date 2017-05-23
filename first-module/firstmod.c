#include <linux/module.h> 
#include <linux/version.h> 

static int start(void) {
    printk("Hello Kernel Log!\n");
    return 0;
}

static void end(void) {
}

module_init(start);
module_exit(end);

MODULE_LICENSE("GPL");
