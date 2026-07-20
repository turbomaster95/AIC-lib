#define _GNU_SOURCE

#include <internal/pal.h>
#include <sys/mman.h>
#include <stdarg.h>

void *mremap(void *old_addr, size_t old_size, size_t new_size, int flags, ...) {
    void *new_addr = NULL;

    if (flags & MREMAP_FIXED) {
        va_list args;
        va_start(args, flags);
        new_addr = va_arg(args, void *);
        va_end(args);
    }

    return pal_mremap(old_addr, old_size, new_size, flags, new_addr);
}
