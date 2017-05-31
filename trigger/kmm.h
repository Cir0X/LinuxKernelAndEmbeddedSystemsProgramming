int kmm_init(void);
void kmm_exit(void);

struct element {
    char *message;
    struct element *next;
};

int printm(const char *, ...);
void enqueue(struct element *, char *);
char * dequeue(struct element *);
