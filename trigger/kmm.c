#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <stdarg.h>
#include "kmm.h"

#define KERNEL_SPACE
#define KMM_DDVR_NAME "KernelModuleMonitor"
#define KMM_CLASS_NAME "KernelModuleMonitorClass"
#define KMM_DEVICE_NAME "kmm"

MODULE_LICENSE("GPL");

// FIFO -----------------------------------------

static struct element * message_log;

void *allocate_memory(int size) {
#ifdef KERNEL_SPACE
    return kmalloc(size, GFP_KERNEL);
#else
    return malloc(size);
#endif
}

void free_memory(void *object) {
#ifdef KERNEL_SPACE
    kfree(object);
#else
    free(object);
#endif
}

struct element *create_element(void) {
    struct element * message_log = (struct element *) allocate_memory(sizeof(struct element));
    message_log->message = NULL;
    message_log->next = NULL;
    return message_log;
}

// copied from:
// https://stackoverflow.com/a/37132824/3014979
char *strdup(char *src) {
    char *str;
    char *p;
    int len = 0;

    while (src[len])
        len++;
    str = allocate_memory(len + 1);
    p = str;
    while (*src)
        *p++ = *src++;
    *p = '\0';
    return str;
}

void enqueue(struct element *message_log, char *message) {
    if (message_log->message == NULL) {
        message_log->message = message;
        message_log->next = create_element();
    } else {
        enqueue(message_log->next, message);
    }
}

char * dequeue(struct element *message_log) {
    if (message_log->message != NULL) {
        char * message = strdup(message_log->message);
        struct element * new_message_log = create_element();
        new_message_log = message_log->next;
        memcpy(message_log, new_message_log, sizeof(struct element));
        free_memory(new_message_log);
        return message;
    } else {
        return NULL;
    }
}
// END FIFO -----------------------------------------

// PRINTM -----------------------------------------
void message_destroy(char *message) {
    free_memory(message);
}

int printm(const char *format, ...) {
    int count = 0;
    char buffer[1024], *message;
    va_list list_ptr;
    va_start(list_ptr, format);
    count = vsnprintf(buffer, sizeof(buffer), format, list_ptr);
    if (count >= sizeof(buffer) - 1) {
        count = sizeof(buffer) - 1;
    }
    message = (char *) allocate_memory((count + 1) * sizeof(char));
    if (message == NULL) {
        va_end (list_ptr);
        return -1;
    }
    strcpy(message, buffer);

    // message_destroy(message);
    enqueue(message_log, message);

    va_end(list_ptr);
    return 0;
}
// END PRINTM -----------------------------------------

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

int kmm_init(void) {
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

    // Initialize FIFO Queue
     message_log = create_element();

    return 0;
}

void kmm_exit(void) {
    cleanup();
}

// module_init(kmm_init);
// module_exit(kmm_exit);
