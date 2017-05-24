#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define DDVR_NAME "KernelModuleMonitor"
#define CLASS_NAME "KernelModuleMonitorClass"
#define DEVICE_NAME "kmm"

static int kmm_open(struct inode *fsde, struct file * mm_entity) {
    printk("KMM: open()\n");
    return 0;
}

static int kmm_close(struct inode *fsde, struct file * mm_entity) {
    printk("KMM: close()\n");
    return 0;
}

static ssize_t kmm_read(struct file *fsde, char *buffer, size_t count, loff_t *long_offset) {
    printk("KMM: read()\n");
    return count;
}

static ssize_t kmm_write(struct file *fsde, const char *buffer, size_t count, loff_t *long_offset) {
    printk("KMM: write()\n");
    return count;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static int kmm_ioctl(struct inode * fsdev, struct file * mm_entity, unsigned int cmd) {
    printk("KMM: ioctl()\n");
    return 0;
}
#else
static long kmm_ioctl(struct file * mm_entitiy, unsigned int cmd, unsigned long arg) {
    printk("KMM: ioctl()\n");
    return 0l;
}
#endif

static struct file_operations kmm_fops = {
    .owner = THIS_MODULE,
    .open = kmm_open,
    .release = kmm_close,
    .read = kmm_read,
    .write = kmm_write,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
    .ioctl = kmm_ioctl,
#else
    .unlocked_ioctl = kmm_ioctl,
#endif
};

dev_t device_number;
struct cdev * cdev_struct;
struct class * class_struct;
struct device * device_struct;

static int start(void) {
    int result;
    int maj;
    int min;

    result = alloc_chrdev_region(&device_number, 0, 1, "dummy_dev");
    printk("alloc_chrdev_region() result = %d\n", result);
    printk("alloc_chrdev_region() device_number = %d\n", device_number);
    maj = MAJOR(device_number);
    min = MINOR(device_number);
    printk("major = %d, minor = %d\n", maj, min);

    cdev_struct = cdev_alloc();
    if (cdev_struct == NULL) {
        printk("KMM: Error allocating cdev\n");
        return 0;
    }

    cdev_init(cdev_struct, &kmm_fops);

    kobject_set_name(&cdev_struct->kobj, DDVR_NAME);
    cdev_struct->owner = THIS_MODULE;

    result = cdev_add(cdev_struct, device_number, 1);
    if (result < 0) {
        printk("KMM: Adding the device failed!\n");
        return 0; 
    }

    class_struct = class_create(THIS_MODULE, CLASS_NAME);
    if (class_struct == NULL) {
        printk("KMM: Error while creating class!\n");
        return 0;
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
    device_struct = device_create(class_struct, NULL, device_number,  DEVICE_NAME);
#else
    device_struct = device_create(class_struct, NULL, device_number, NULL, DEVICE_NAME);
#endif

    if (IS_ERR(class_struct) || device_struct == NULL) {
        printk("KMM: class registration failed (udev)\n");
        return 0;
    }

    return 0;
}

static void end(void) {
    if (cdev_struct != NULL) {
        cdev_del(cdev_struct);
        unregister_chrdev_region(device_number, 1);
        /*class_destroy();*/
    }
}

module_init(start);
module_exit(end)
