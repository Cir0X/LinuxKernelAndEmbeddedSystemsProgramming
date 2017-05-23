#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fifo-queue.h"

//#define KERNEL_SPACE

struct element {
    char *message;
    struct element *next;
};

void *allocate_memory(int size) {
#ifdef KERNEL_SPACE
    return kmalloc(size);
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

struct element * create_element() {
    struct element * message_log = (struct element *) allocate_memory(sizeof(struct element));
    message_log->message = NULL;
    message_log->next = NULL;
    return message_log;
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

int main(void) {
    struct element * message_log = create_element();
    char *message;

    printf("Initialized vars\n");

    enqueue(message_log, "Hello");
    enqueue(message_log, "World!");

    printf("returned message = %s\n", dequeue(message_log));
    printf("returned message = %s\n", dequeue(message_log));
    printf("returned message = %s\n", dequeue(message_log));

    free_memory(message_log);
    return 0;
}
