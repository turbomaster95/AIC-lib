#include <internal/syscall.h>
#include <stdio.h>
#include <string.h>

void print_str(const char *s) {
    // Change SYS_write to __NR_write
    __syscall3(__NR_write, 1, (long)s, strlen(s));
}
