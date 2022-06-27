#include "operations.h"
#include "stack.h"
#include "error.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

enum _processing_state {READING_INT, READING_FLOAT, READING_SYMBOL};


bool process_in_float = false;
char error[128];

void set_process_in_float(bool process) {
    process_in_float = process;
}

void print_error() {
    printf("Error while processing expression: %s", error);
}

#define ERRORF(pattern, ...) {\
    snprintf(error, 128, pattern, __VA_ARGS__);\
    return 1;\
}

#define ERROR(text) {\
    snprintf(error, 128, text);\
    return 1;\
}

#define generate_op_function(name, op)\
 static struct stack_entry name(struct stack_entry* a, struct stack_entry* b) {\
    struct stack_entry e;\
    if (a->type == FloatStackEntry || b->type == FloatStackEntry || process_in_float) {\
        e.type = FloatStackEntry;\
        e.data.f = (a->type == FloatStackEntry? a->data.f : a->data.i) op\
           (b->type == FloatStackEntry? b->data.f : b->data.i);\
    } else {\
        e.type = IntStackEntry;\
        e.data.i = a->data.i op b->data.i;\
    } \
    return e;\
}


generate_op_function(sum_entries, +)
generate_op_function(diff_entries, -)
generate_op_function(mult_entries, *)
generate_op_function(div_entries, /)

static bool _is_char_valid(char ch) {
    for (unsigned int i = 0; i < strlen(ALLOWED_CHARS); i++) {
        if (ch == ALLOWED_CHARS[i] || ch == EOF) {
            return true;
        }
    }
    return false;
}

static bool is_op(char ch) {
    return ch=='+' || ch=='-' || ch=='*' || ch=='/';
}

static int priority(char ch) {
    if (ch == '+' || ch == '-') {
        return 0;
    }
    if (ch == '/' || ch == '*') {
        return 1;
    }
    if (ch == -'-' || ch == -'+') {
        return 2;
    }
    return -1;
}

int process_input_stream(FILE *stream, struct stack **returned_queue)
{
    int err = allock_stack(STACK_SIZE, returned_queue);
    struct stack* processing_stack;
    err |= allock_stack(STACK_SIZE, &processing_stack);
    if (err) {
        ERROR("Error while allocating stack for processing.");
    }

    char current_char;
    unsigned int current_int;
    float current_float;
    unsigned int float_length = 0;
    enum _processing_state state = READING_SYMBOL;
    unsigned int char_count = 0;
    bool can_be_unary = true;
    while (1) {
        current_char = fgetc(stream);
        char_count ++;

        if (char_count > MAX_INPUT_LENGTH) {
            ERROR("Expression is longer then max input length");
        }

        if (!_is_char_valid(current_char)) {
            ERRORF("Invalid character <%c> in position %d", current_char, char_count);
        }

        if (current_char >= '0' && current_char <= '9') {
            can_be_unary = false;
            if (state == READING_SYMBOL) {
                current_int = current_char - '0';
                state = READING_INT;
                continue;
            }

            if (state == READING_INT) {
                if (current_int * 10 + (current_char - '0') < current_int) {
                    ERRORF("Wrong character <%c> while reading int at pos %d", current_char, char_count);
                }
                current_int = current_int * 10 + (current_char - '0');
                continue;
            }
            float_length ++;
            current_float = (current_char - '0') * (0.1 * float_length) + current_float;
            continue;
        }

        if (current_char == '.') {
            can_be_unary = false;
            if (state == READING_INT) {
                current_float = current_int;
                state = READING_FLOAT;
                float_length = 0;
                continue;
            }
            if (state == READING_FLOAT) {
                ERRORF("Error while float. Second '.' symbol on pos %d", char_count);
            }
            current_float = 0;
            state = READING_FLOAT;
            float_length = 0;
            continue;
        }

        if (state == READING_INT) {
            err = add_i(current_int, *returned_queue);
            if (err) {
                ERROR("Processing stack is too small");
            }
            state = READING_SYMBOL;
        }

        if (state == READING_FLOAT) {
            err = add_f(current_float, *returned_queue);
            if (err) {
                ERROR("Processing stack is too small");
            }
            state = READING_SYMBOL;
        }

        if (current_char == '(') {
            can_be_unary = true;
            err = add_ch('(', processing_stack);
            if (err) {
                ERROR("Processing stack is too small");
            }
            continue;
        }

        if (current_char == ')') {
            can_be_unary = false;
            struct stack_entry entry;
            err = pop(&entry, processing_stack);
            if (err) {
                ERRORF("Found ')', but not found '(' before it. pos %d", char_count);
            }
            while(!(entry.type == CharStackEntry && entry.data.ch == '(')) {
                add_entry(entry, *returned_queue);
                err = pop(&entry, processing_stack);
                if (err) {
                    ERRORF("Found ')', but not found '(' before it. pos %d", char_count);
                }
            }
            continue;
        }

        if (current_char == '\n' || current_char == EOF) {
            if (char_count == 1) {
                return EOF;
            }
            struct stack_entry entry;
            while(!pop(&entry, processing_stack)) {
                add_entry(entry, *returned_queue);
            }
            break;
        }

        if (is_op(current_char)) {
            if (can_be_unary) {
                if (current_char == '-' || current_char == '+') {
                    current_char = -current_char;
                } else {
                    ERRORF("Found unary usage of op <%c>, but it cannot be unary. pos %d", current_char, char_count);
                }
            }
            can_be_unary = true;
            struct stack_entry entry;
            err = get(&entry, processing_stack);
            if (err) {
                err = add_ch(current_char, processing_stack);
                if (err) {
                    ERROR("Processing stack is too small");
                }
                continue;
            }
            while (priority(current_char) <= priority(entry.data.ch)) {
                pop(&entry, processing_stack);
                add_entry(entry, *returned_queue);

                err = get(&entry, processing_stack);
                if (err) {
                    break;
                }
            }

            err = add_ch(current_char, processing_stack);
            if (err) {
                ERROR("Processing stack is too small");
            }
        }
    }
    free_stack(processing_stack);
    reverse(*returned_queue);
    return 0;
}

int count_inverse(struct stack_entry* return_entry, struct stack* queue)
{
    struct stack* processing_stack;
    int err = allock_stack(STACK_SIZE, &processing_stack);
    if (err) {
        ERROR("Cannot allocate stack to process expression");
    }

    struct stack_entry current;
    while (pop(&current, queue) == 0) {
        if (current.type == IntStackEntry || current.type == FloatStackEntry) {
            add_entry(current, processing_stack);
            continue;
        }
        switch (current.data.ch) {
            case '+': {
                struct stack_entry first, second;
                int err = pop(&first, processing_stack);
                err |= pop(&second, processing_stack);
                if (err) {
                    ERROR("No argumets for op '+'")
                }
                add_entry(sum_entries(&first, &second), processing_stack);
                continue;
            }
            case '-': {
                struct stack_entry first, second;
                int err = pop(&first, processing_stack);
                err |= pop(&second, processing_stack);
                if (err) {
                    ERROR("No argumets for op '-'")
                }
                add_entry(diff_entries(&second, &first), processing_stack);
                continue;
            }
            case '*': {
                struct stack_entry first, second;
                int err = pop(&first, processing_stack);
                err |= pop(&second, processing_stack);
                if (err) {
                    ERROR("No argumets for op '*'")
                }
                add_entry(mult_entries(&first, &second), processing_stack);
                continue;
            }
            case '/': {
                struct stack_entry first, second;
                int err = pop(&first, processing_stack);
                err |= pop(&second, processing_stack);
                if (err) {
                    ERROR("No argumets for op '/'")
                }
                add_entry(div_entries(&second, &first), processing_stack);
                continue;
            }
            case -'-': {
                struct stack_entry first;
                int err = pop(&first, processing_stack);
                if (err) {
                    ERROR("No argumets for op unary '-'")
                }
                if (first.type == FloatStackEntry) {
                    first.data.f = -first.data.f;
                    add_entry(first, processing_stack);
                } else {
                    first.data.i = -first.data.i;
                    add_entry(first, processing_stack);
                }
                continue;
            }
            case -'+': {
                continue;
            }
        }
    }
    err = pop(return_entry, processing_stack);
    if (err) {
        ERROR("No result in stack")
    }
    if (processing_stack->current_size > 0) {
        ERROR("Wrong operands number")
    }
    free_stack(processing_stack);
    return 0;
}

int execute_expression(FILE *stream, struct stack_entry *return_entry) {
    struct stack* queue;
    int err = process_input_stream(stream, &queue);
    if (err) {
        return err;
    }
    err = count_inverse(return_entry, queue);
    free_stack(queue);
    return err;
}