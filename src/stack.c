#include "stack.h"
#include <stdlib.h>
#include <string.h>

int allock_stack(unsigned int size, struct stack** stack_p)
{
    struct stack_entry* array = (struct stack_entry *) malloc(size * sizeof(struct stack_entry));
    *stack_p = (struct stack*) malloc(sizeof(struct stack));
    if (array == NULL || *stack_p == NULL) {
        return 1;
    }
    (**stack_p).stack_array = array;
    (**stack_p).current_size = 0;
    (**stack_p).max_size = size;
    return 0;
}

void free_stack(struct stack *stack_p)
{
    free(stack_p->stack_array);
    free(stack_p);
}

int add_i( int data, struct stack * stack_p )
{
    if (stack_p->current_size == stack_p->max_size) {
        return 1;
    }
    struct stack_entry* entry = &stack_p->stack_array[stack_p->current_size];
    entry->type = IntStackEntry;
    entry->data.i = data;

    stack_p->current_size++;
    return 0;
}

int add_ch( char data, struct stack * stack_p )
{
    if (stack_p->current_size == stack_p->max_size) {
        return 1;
    }
    struct stack_entry* entry = &stack_p->stack_array[stack_p->current_size];
    entry->type = CharStackEntry;
    entry->data.ch = data;
    stack_p->current_size++;
    return 0;
}

int add_f( float data, struct stack * stack_p )
{
    if (stack_p->current_size == stack_p->max_size) {
        return 1;
    }
    struct stack_entry* entry = &stack_p->stack_array[stack_p->current_size];
    entry->type = FloatStackEntry;
    entry->data.f = data;
    stack_p->current_size++;
    return 0;
}

int add_entry(struct stack_entry data, struct stack * stack_p)
{
    if (stack_p->current_size == stack_p->max_size) {
        return 1;
    }

    struct stack_entry* entry = &stack_p->stack_array[stack_p->current_size];
    entry->type = data.type;
    entry->data = data.data;
    stack_p->current_size++;
    return 0;
}

int pop(struct stack_entry *entry, struct stack *stack_p)
{
    if (stack_p->current_size <= 0) {
        return 1;
    }
    struct stack_entry* curr_entry = &(stack_p->stack_array[stack_p->current_size - 1]);
    entry->type = curr_entry->type;
    entry->data = curr_entry->data;
    stack_p->current_size -= 1;
    return 0;
}

int get(struct stack_entry *entry, struct stack *stack_p)
{
    if (stack_p->current_size <= 0) {
        return 1;
    }
    struct stack_entry * curr_entry = &(stack_p->stack_array[stack_p->current_size - 1]);
    entry->type = curr_entry->type;
    entry->data = curr_entry->data;
    return 0;
}

void reverse(struct stack *stack_p)
{
    for (unsigned int i = 0; i < stack_p->current_size / 2; i ++) {
        struct stack_entry s;
        s = stack_p->stack_array[stack_p->current_size - 1 - i];
        stack_p->stack_array[stack_p->current_size - 1 - i] = stack_p->stack_array[i];
        stack_p->stack_array[i] = s;
    }
}
