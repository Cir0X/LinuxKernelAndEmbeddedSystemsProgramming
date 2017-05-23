#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>

static int start(void) {
    dev_t dev_no;
    int result;
    int maj;
    int min;
    result = alloc_chrdev_region(&dev_no, 0, 1, "dummy_dev");
    printk("alloc_chrdev_region() result = %d\n", result);
    printk("alloc_chrdev_region() dev_no = %d\n", dev_no);
    maj = MAJOR(dev_no);
    min = MINOR(dev_no);
    printk("major = %d, minor = %d\n", maj, min);

    cdev_alloc();
    return 0;
}

static void end(void) {
    cdev_del();
}

module_init(start);
module_exit(end)
