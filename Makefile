CC=gcc
CFLAGS=-Wall -Iinclude -O
LD=ld
LDFLAGS=

main: src/main.c cmdline operations
	$(CC) src/main.c $(CFLAGS) -c -o build/main.o;
	$(CC) build/main.o build/stack.o build/operations.o build/cmdline.o $(CFLAGS) -o build/calc

cmdline: src/cmdline.c include/cmdline.h
	$(CC) src/cmdline.c $(CFLAGS) -c -o build/cmdline.o

test: test_operations

test_operations: test_operations_build
	$(CC) build/stack.o build/operations.o build/operation_test.o $(CFLAGS) -o build/operation_test;
	./build/operation_test

test_operations_build: test/operation_test.c operations
	$(CC) test/operation_test.c $(CFLAGS) -c -o build/operation_test.o

operations: include/operations.h src/operations.c stack init
	$(CC) src/operations.c $(CFLAGS) -c -o build/operations.o

stack: include/stack.h src/stack.c init
	$(CC) src/stack.c $(CFLAGS) -c -o build/stack.o

init:
	-mkdir -p build

clean:
	-rm -rf build
