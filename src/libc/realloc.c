#include <stdlib.h>

#undef realloc

extern void *__libc_realloc(void *, size_t);

void *realloc(void *p, size_t n) {
    return __libc_realloc(p, n);
}
