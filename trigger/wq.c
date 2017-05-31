#include <linux/module.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include "./kmm.h"

MODULE_LICENSE("GPL");

static void handle_work(struct work_struct *);

static struct workqueue_struct * workqueue;
static DECLARE_DELAYED_WORK(work, handle_work);

static int counter = 0;
static long thirty_seconds;

static void handle_work(struct work_struct *delayed_work) {
    printk("KMM Test Module: Doing work...\n");
    printm("Working for %d seconds\n", counter * 30);

    workqueue = create_singlethread_workqueue("kmm");
    thirty_seconds = msecs_to_jiffies(30000);
    queue_delayed_work(workqueue, &work, thirty_seconds);
}

static void cleanup(void) {
    int result;
    result = cancel_delayed_work(&work);
    if (result == 0) {
        flush_workqueue(workqueue);
    }
    destroy_workqueue(workqueue);

    kmm_exit();
    printk("KMM Test Module: Terminating...\n");
}

static int start(void) {
    int result;
    printk("KMM Test Module: Entering module initialization...\n");
    result = kmm_init();

    if (result != 0) {
        printk("KMM Test Module: Test module KMM initialization failed...\n");
        cleanup();
        return 0;
    }

    workqueue = create_singlethread_workqueue("kmm");
    thirty_seconds = msecs_to_jiffies(30000);

    queue_delayed_work(workqueue, &work, thirty_seconds);

    return 0;
}

static void end(void) {
    cleanup();
}

module_init(start);
module_exit(end);
