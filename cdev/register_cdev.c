#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/io.h>

#define KMM_DDVR_NAME "KernelModuleMonitor"
#define KMM_CLASS_NAME "KernelModuleMonitorClass"
#define KMM_DEVICE_NAME "kmm"

MODULE_LICENSE("GPL");

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
    .read = kmm_
    .write = kmm_write,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
    .ioctl = kmm_ioctl,
#else
    .unlocked_ioctl = kmm_ioctl,
#endif
};

dev_t device_number;
static struct cdev * cdev_struct = NULL;
static struct class * class_struct = NULL;
static struct device * device_struct = NULL;

void cleanup(void) {
    device_destroy(class_struct, device_number);
    if (class_struct != NULL) {
        class_destroy(class_struct);
    }
    if (cdev_struct != NULL) {
        cdev_del(cdev_struct);
        kobject_put(&cdev_struct->kobj);
        cdev_struct = NULL;
        unregister_chrdev_region(device_number, 1);
    }
}

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
        cleanup();
        return 0;
    }

    cdev_init(cdev_struct, &kmm_fops);

    kobject_set_name(&cdev_struct->kobj, KMM_DDVR_NAME);
    cdev_struct->owner = THIS_MODULE;

    result = cdev_add(cdev_struct, device_number, 1);
    if (result < 0) {
        printk("KMM: Adding the device failed!\n");
        cleanup();
        return 0;
    }

    class_struct = class_create(THIS_MODULE, KMM_CLASS_NAME);
    if (class_struct == NULL) {
        printk("KMM: Error while creating class!\n");
        cleanup();
        return 0;
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
    device_struct = device_create(class_struct, NULL, device_number,  KMM_DEVICE_NAME);
#else
    device_struct = device_create(class_struct, NULL, device_number, NULL, KMM_DEVICE_NAME);
#endif

    if (IS_ERR(class_struct) || device_struct == NULL) {
        printk("KMM: class registration failed (udev)\n");
        cleanup();
        return 0;
    }

    return 0;
}

static void end(void) {
    cleanup();
}

module_init(start);
module_exit(end)
