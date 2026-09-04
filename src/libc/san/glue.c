#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <errno.h>

void drt_arch_print_string(const char *str) {
    volatile unsigned char *uart = (volatile unsigned char *)0x3F8;
    while (*str) {
        *uart = *str++;
        for (volatile int i = 0; i < 100; i++) __asm__ volatile("" ::: "memory");
    }
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

uint64_t drt_arch_get_time_ns(void) {
    unsigned lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

void drt_arch_yield(void) {
    __asm__ volatile ("pause" ::: "memory");
}
