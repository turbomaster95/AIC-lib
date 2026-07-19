#include <internal/pal.h>
#include <sys/mman.h>

int mprotect(void *addr, size_t length, int prot) {
    return (int)pal_mprotect(addr, length, prot);
}
