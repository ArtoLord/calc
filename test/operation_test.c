#include "operations.h"
#include "stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
                            if (message) return message; } while (0)
int tests_run;

static char* test_stack()
{
    struct stack* stack_p;
    int err = allock_stack(4, &stack_p);
    mu_assert("Cannot allock stack", err == 0);
    err = add_i(1, stack_p);
    mu_assert("Cannot add to stack", err == 0);
    err = add_ch('2', stack_p);
    mu_assert("Cannot add to stack", err == 0);
    err = add_f(3.14, stack_p);
    mu_assert("Cannot add to stack", err == 0);

    struct stack_entry entry;
    entry.type = IntStackEntry;
    entry.data.i = 4;

    err = add_entry(entry, stack_p);
    mu_assert("Cannot add to stack", err == 0);

    err = add_i(42, stack_p);
    mu_assert("Can add to full stack", err != 0);

    pop(&entry, stack_p);
    mu_assert("Pop 1: entry.data.i != 4", entry.data.i == 4);

    pop(&entry, stack_p);
    mu_assert("Pop 2: entry.type != FloatStackEntry", entry.type == FloatStackEntry);

    pop(&entry, stack_p);
    mu_assert("Pop 3: entry.data.ch != '2'", entry.data.ch == '2');

    pop(&entry, stack_p);
    mu_assert("Pop 4: entry.data.i != 1", entry.data.i == 1);

    add_i(42, stack_p);
    get(&entry, stack_p);
    mu_assert("Get: entry.data.i != 42", entry.data.i == 42);

    free_stack(stack_p);

    return 0;
}

static char* test1()
{
    int a = rand();
    char buffer[64];
    sprintf(buffer,"/tmp/test%d.txt", a);
    FILE* f = fopen(buffer, "w");
    fprintf(f, "(1+2*(3+4)-1)/6");
    fclose(f);
    f = fopen(buffer, "r");
    struct stack* queue;
    int err = process_input_stream(f, &queue);
    mu_assert("Error while processing stream", err == 0);
    buffer[0] = '\0';
    char* str = malloc(64);
    str[0] = '\0';
    struct stack_entry entry;
    while ((err = pop(&entry, queue) == 0)) {
        strcpy(str, buffer);
        if (entry.type == IntStackEntry) {
            sprintf(buffer, "%s %d", str, entry.data.i);
        } else if (entry.type == FloatStackEntry) {
            sprintf(buffer, "%s %f", str, entry.data.f);
        } else {
            sprintf(buffer, "%s %c", str, entry.data.ch);
        }
    }

    char* true_exit = " 1 2 3 4 + * + 1 - 6 /";

    sprintf(str, "Assert: %s != %s", true_exit, buffer);

    mu_assert(str, strcmp(true_exit, buffer) == 0);
    fclose(f);
    return 0;
}

static char* test_unary()
{
    int a = rand();
    char buffer[64];
    sprintf(buffer,"/tmp/test%d.txt", a);
    FILE* f = fopen(buffer, "w");
    fprintf(f, "1+(-1)");
    fclose(f);
    f = fopen(buffer, "r");
    struct stack* queue;
    int err = process_input_stream(f, &queue);
    mu_assert("Error while processing stream", err == 0);
    buffer[0] = '\0';
    char* str = malloc(64);
    str[0] = '\0';
    struct stack_entry entry;
    while ((err = pop(&entry, queue) == 0)) {
        strcpy(str, buffer);
        if (entry.type == IntStackEntry) {
            sprintf(buffer, "%s %d", str, entry.data.i);
        } else if (entry.type == FloatStackEntry) {
            sprintf(buffer, "%s %f", str, entry.data.f);
        } else {
            sprintf(buffer, "%s %c", str, entry.data.ch);
        }
    }

    char true_exit[16];
    sprintf(true_exit, " 1 1 %c +", -'-');

    sprintf(str, "Assert: %s != %s", true_exit, buffer);

    mu_assert(str, strcmp(true_exit, buffer) == 0);
    fclose(f);
    return 0;
}

char * test_count_simple(){
    int a = rand();
    char buffer[64];
    sprintf(buffer,"/tmp/test%d.txt", a);
    FILE* f = fopen(buffer, "w");
    fprintf(f, "(1+2*(3+4)-1)/6");
    fclose(f);
    f = fopen(buffer, "r");
    struct stack* queue;
    int err = process_input_stream(f, &queue);
    mu_assert("Error while processing stream", err == 0);

    struct stack_entry ret;

    err = count_inverse(&ret, queue);
    mu_assert("Error while counting result", err == 0);

    mu_assert("Assert error: ret.i != 2", ret.data.i == 2);

    free_stack(queue);
    fclose(f);
    return 0;
}

char * test_count_unary(){
    int a = rand();
    char buffer[64];
    sprintf(buffer,"/tmp/test%d.txt", a);
    FILE* f = fopen(buffer, "w");
    fprintf(f, "(1+2*(3-(-4))+(-1))/6");
    fclose(f);
    f = fopen(buffer, "r");
    struct stack* queue;
    int err = process_input_stream(f, &queue);
    mu_assert("Error while processing stream", err == 0);

    struct stack_entry ret;

    err = count_inverse(&ret, queue);
    mu_assert("Error while counting result", err == 0);

    mu_assert("Assert error: ret.i != 2", ret.data.i == 2);

    free_stack(queue);
    fclose(f);
    return 0;
}

char * test_count_float(){
    int a = rand();
    char buffer[64];
    sprintf(buffer,"/tmp/test%d.txt", a);
    FILE* f = fopen(buffer, "w");
    fprintf(f, "(3.+3)/2");
    fclose(f);
    f = fopen(buffer, "r");
    struct stack* queue;
    int err = process_input_stream(f, &queue);
    mu_assert("Error while processing stream", err == 0);

    struct stack_entry ret;

    err = count_inverse(&ret, queue);
    mu_assert("Error while counting result", err == 0);

    mu_assert("Assert error: ret.f != 3", ret.data.f > 2.99999 && ret.data.f < 3.00001);

    free_stack(queue);
    fclose(f);
    return 0;
}

static char * all_tests() {
     mu_run_test(test_stack);
     mu_run_test(test1);
     mu_run_test(test_unary);
     mu_run_test(test_count_simple);
     mu_run_test(test_count_unary);
     mu_run_test(test_count_float);
     return 0;
}


int main() {
    char *result = all_tests();
     if (result != 0) {
         printf("%s\n", result);
     }
     else {
         printf("ALL TESTS PASSED\n");
     }
     printf("Tests run: %d\n", tests_run);
 
     return result != 0;
}