#include <internal/pal.h>
#include <sys/mman.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
	return pal_mmap(addr, length, prot, flags, fd, offset);
}

