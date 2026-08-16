#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

_Noreturn void __assert_fail(const char *expr, const char *file, int line, const char *func) {
    if (func) {
        fprintf(stderr, "Assertion failed: %s (%s: %s: %d)\n", expr, file, func, line);
    } else {
        fprintf(stderr, "Assertion failed: %s (%s: %d)\n", expr, file, line);
    }
    abort();
}
