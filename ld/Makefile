# Makefile for aic-ld.so
#
# Build the dynamic linker:
#   make
#
# Test with a simple program:
#   make test
#
# Clean:
#   make clean

CC = gcc
CFLAGS = -nostdlib -fno-stack-protector -fno-builtin          -fno-asynchronous-unwind-tables -fPIC          -O2 -Wall -Wextra -Werror -Wno-unused
LDFLAGS = -shared -Wl,-e,_start

all: aic-ld.so

aic-ld.so: aic-ld.o aic-ld-start.o
	$(CC) $(LDFLAGS) -o $@ $^

aic-ld.o: aic-ld.c syscall.h
	$(CC) $(CFLAGS) -c -o $@ $<

aic-ld-start.o: aic-ld-start.s
	$(CC) -c -o $@ $<

# Test: try to run /bin/echo via our linker
test: aic-ld.so
	@echo "=== Testing as direct invoker ==="
	./aic-ld.so /bin/echo "Hello from aic-ld!"
	@echo ""
	@echo "=== Testing as PT_INTERP ==="
	@echo "(Build a test program with -Wl,--dynamic-linker=./aic-ld.so)"

# Build a test shared library
testlib.so: testlib.c
	$(CC) -fPIC -shared -o $@ $<

testlib.c:
	@echo 'const char* hello() { return "Hello from testlib!"; }' > $@

clean:
	rm -f aic-ld.so aic-ld.o aic-ld-start.o testlib.so testlib.c

.PHONY: all test clean
