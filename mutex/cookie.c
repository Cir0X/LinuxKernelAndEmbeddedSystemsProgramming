#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched.h>
// #include <linux/completion.h>

#define MUTEXMODE // remove to create a race condition

// Mutex = spezialisierter Semaphore

static struct task_struct *thread_task1;
static struct task_struct *thread_task2;

static DECLARE_COMPLETION(on_exit);

#ifdef MUTEXMODE
static DEFINE_MUTEX(CookieDispatcher);
#endif

static void get_cookie(void *data, int numberOfCookies) {
    static int cookiesInTheBox = 10;
#ifdef MUTEXMODE
    mutex_lock(&CookieDispatcher);
#endif
    if ((cookiesInTheBox - numberOfCookies) <= 0) {
        printk("get_cookie: Will hand over %d cookies to Thread %s\n", numberOfCookies, (char *) data);
        msleep(1000); // Getting the cookies takes some time
        cookiesInTheBox -= numberOfCookies;
    } else {
        printk("Not enough cookies available: %d in the box\n", cookiesInTheBox);
#ifdef MUTEXMODE
    mutex_unlock(&CookieDispatcher);
#endif
        printk("Leaving >>get_cookie<< with %d cookies in the box\n", cookiesInTheBox);
    }
}

static int threadWorker(void *data) {
    printk("Hello from threadWorker!\n");
    get_cookie(data, 10);
    return 0;
}

int cbf(void * data) {
    int timeout;
    wait_queue_head_t(&waitQueue),
    init_waitqueue_head(%waitQueue);
    allow_signal(SIGTERM);
    while (1) {
        timeout = 2 * HZ; // 2 secs
        get_cookie(data, 2);
        timeout = wait_event_interruptible_timeout(waitQueue, (timeout == 0), timeout);
        if (timeout == -ERESTARTSYS) {
            break;

    }

    complete_and_exit(&on_exit, 0);
    return 0;
}

static int start(void) {
    printk("Cookie Module Loaded\n");
    thread_task1 = kthread_run(threadWorker, "Hello World", "CookieThread");// kthread_run, wake_up
    thread_task2 = kthread_run(threadWorker, "Hello World", "CookieThread");
    return 0;
}

static void end(void) {
    printk("Cookie Module UnLoaded\n");

    kill_pid(task_pid(thread1), SIGTERM, 1);
    kill_pid(task_pid(thread2), SIGTERM, 1);
    wait_for_completion(&on_exit);
    wait_for_completion(&on_exit);
}

module_init(start);
module_exit(end);

MODULE_LICENSE("GPL");
