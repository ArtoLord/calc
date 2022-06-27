#include "operations.h"
#include "cmdline.h"
#include "stack.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(int argc, char** argv) {
    struct gengetopt_args_info ai;
    if (cmdline_parser(argc, argv, &ai) != 0) {
        cmdline_parser_print_help();
        return 1;
    }
    FILE* file_to_read;

    if (ai.float_flag) {
        set_process_in_float(true);
    }

    if (ai.filename_given) {
        file_to_read = fopen(ai.filename_arg, "r");
        char buf[32];
        sprintf(buf, "Error while opening file %s: ", ai.filename_arg);
        if (file_to_read == NULL) {
            perror(buf);
            return 1;
        }
    } else if (ai.expression_given) {
        file_to_read = fmemopen(ai.expression_arg, strlen(ai.expression_arg), "r");
    } else {
        file_to_read = stdin;
    }

    struct stack_entry entry;

    int err = 0;
    while (!err) {
        err = execute_expression(file_to_read, &entry);
        if (err == EOF) {
            return 0;
        }
        if (err) {
            print_error();
            continue;
        }
        if (entry.type == IntStackEntry) {
            printf("%d\n", entry.data.i);
        } else {
            printf("%f\n", entry.data.f);
        }
    }

    return 0;
}