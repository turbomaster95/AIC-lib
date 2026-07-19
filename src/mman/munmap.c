#include <internal/pal.h>
#include <sys/mman.h>

int munmap(void *addr, size_t length) {
     return (int)pal_munmap(addr, length);
}
