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

static unsigned char term_flag = 0x00;

static void handle_work(struct work_struct *delayed_work) {
    int result;
    printk("KMM Test Module: Doing work...\n");
    printm("Working for %d seconds\n", counter * 30);

    if (term_flag == 0x00) {
        // Nochmal die workqueue initialisieren? Muss das sein?
        // repeat_delayed_work()? restart_delayed_work()?
        workqueue = create_singlethread_workqueue("kmm");
        thirty_seconds = msecs_to_jiffies(30000);
        result = queue_delayed_work(workqueue, &work, thirty_seconds);
        if (result != 1) {
            printk("KMM fatal error: handle_work() couldn't add task to workqueue\n");
        }
    }
}

static void cleanup(void) {
    int result;
    term_flag = 0x01;
    result = cancel_delayed_work(&work);
    if (result == 0) { // task is running
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
