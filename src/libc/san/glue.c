#define _GNU_SOURCE
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <internal/pal.h>
#include <time.h>

uint64_t drt_arch_get_time_ns(void) {
    struct timespec ts;
    pal_clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000000000ULL) + (uint64_t)ts.tv_nsec;
}

void drt_arch_yield(void) {
    pal_sched_yield();
}

void drt_arch_print_string(const char *str) {
	puts(str);
}

_Noreturn void drt_arch_abort(void) {
    abort();
}

int drt_arch_map_shadow_memory(uintptr_t addr, size_t size) {
    if (addr == 0) {
        addr = 0x7fff8000;
    }

    if (size == 0) {
        size = 0x100000000000ULL - addr;
    }

    void *p = mmap((void *)addr, size,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE | MAP_FIXED_NOREPLACE,
                   -1, 0);

    if (p == MAP_FAILED) {
        p = mmap((void *)addr, size,
                 PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE | MAP_FIXED,
                 -1, 0);
    }

    if (p == MAP_FAILED) {
        return -1;
    }

    return 0;
}
