#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include "variadic-function.h"

void message_destroy(char *message) {
    free(message);
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
    message = (char *) malloc((count + 1) * sizeof(char));
    if (message == NULL) {
        va_end (list_ptr);
        return -1;
    }
    strcpy(message, buffer);

    // Start test
    printf("Function printm() called\n");
    printf("Number of characters collected: %d\n", count);
    printf("String is: %s\nSize: %lu\n", message, strlen(message));
    //printf(buffer);
    // End test

    message_destroy(message);

    va_end(list_ptr);
    return 0;
}

int main(void) {
    printm("Number: %d, Text: %s\n", 10, "Hi");
    return EXIT_SUCCESS;
}
