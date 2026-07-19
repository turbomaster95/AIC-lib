#include <internal/pal.h>
#include <sys/mman.h>

int madvise(void *addr, size_t length, int advice) {
	return (int)pal_madvise(addr, length, advice);
}
