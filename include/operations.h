#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdbool.h>
#include <stdio.h>
#include "stack.h"

#define MAX_INPUT_LENGTH 256
#define ALLOWED_CHARS "1234567890+-*/().\n\0"

int process_input_stream(FILE* stream, struct stack** returned_queue);
int count_inverse(struct stack_entry* return_entry, struct stack* queue);

int execute_expression(FILE* stream, struct stack_entry* return_entry);

void set_process_in_float(bool process);
void print_error();

#endif