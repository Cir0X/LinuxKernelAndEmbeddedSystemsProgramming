#include <linux/module.h>
#include <linux/init.h>
#include <linux/workqueue.h>
/* #include "./kmm.h" */

MODULE_LICENSE("GPL");

void handle_work(struct work_struct *);

struct workqueue_struct * workqueue;
DECLARE_DELAYED_WORK(work, handle_work);

/* static int counter = 0; */
long thirty_seconds;

unsigned char term_flag = 0x00;


extern int printm(const char *format, ...);
int (*kmm_printm)(const char * format, ...);

void handle_work(struct work_struct *delayed_work) {
    int result;
    printk("KMM Test Module: Doing work...\n");
    //    printm("Working for %d seconds\n", counter * 30);

    if (term_flag == 0x00) {
        // Nochmal die workqueue initialisieren? Muss das sein?
        // repeat_delayed_work()? restart_delayed_work()?
        workqueue = create_singlethread_workqueue("kmm_trigger");
        thirty_seconds = msecs_to_jiffies(5000);
        result = queue_delayed_work(workqueue, &work, thirty_seconds);
        if (result != 1) {
            printk("KMM fatal error: handle_work() couldn't add task to workqueue\n");
        }
    }
}

void cleanup(void) {
    int result;
    term_flag = 0x01;
    result = cancel_delayed_work(&work);
    if (result == 0) { // task is running
        flush_workqueue(workqueue);
    }
    destroy_workqueue(workqueue);

    /* kmm_exit(); */


    if (kmm_printm != NULL) {
        /* symbol_put(kmm_put); */
    }
    
    printk("KMM Test Module: Terminating...\n");
}

int start(void) {
    /* int result; */
    printk("KMM Test Module: Entering module initialization...\n");
    
    kmm_printm = symbol_get(printm);

    /* result = kmm_init(); */

    /* if (result != 0) { */
    /*     printk("KMM Test Module: Test module KMM initialization failed...\n"); */
    /*     cleanup(); */
    /*     return 0; */
    /* } */

    workqueue = create_singlethread_workqueue("kmm_trigger");
    thirty_seconds = msecs_to_jiffies(5000);

    queue_delayed_work(workqueue, &work, thirty_seconds);

    return 0;
}

void end(void) {
    cleanup();
}

module_init(start);
module_exit(end);
