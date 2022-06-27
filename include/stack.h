#ifndef STACK_H
#define STACK_H

#define STACK_SIZE 1024

enum stack_entry_t {
    IntStackEntry,
    CharStackEntry,
    FloatStackEntry
};

union stack_data {
    int   i;
    char  ch;
    float f;
};

struct stack_entry {
    enum stack_entry_t type;
    union stack_data data;
};

struct stack {
    struct stack_entry* stack_array;
    unsigned int        current_size;
    unsigned int        max_size;
};

int allock_stack(unsigned int size, struct stack** stack_p);

int add_i(int, struct stack*);
int add_ch(char, struct stack*);
int add_f(float, struct stack*);
int add_entry(struct stack_entry, struct stack*);

int pop(struct stack_entry* entry, struct stack* stack_p);
int get(struct stack_entry *entry, struct stack *stack_p);
void reverse(struct stack* stack_p);

void free_stack(struct stack* stack_p);

#endif