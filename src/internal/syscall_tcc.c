#include "syscall.h"

// This creates a real, linkable function that Clang will build
long tcc_syscall_gen(long n, long a, long b, long c, long d, long e, long f) {
    return __syscall_gen(n, a, b, c, d, e, f);
}
