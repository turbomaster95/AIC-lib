#ifndef MALLOC_GLUE_H
#define MALLOC_GLUE_H

#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <elf.h>
#include <string.h>
#include <internal/syscall.h>

#define a_cas(p, t, s) __sync_val_compare_and_swap(p, t, s)

#define a_store(p, v) __atomic_store_n(p, v, __ATOMIC_RELEASE)

#define a_and(p, v) __sync_fetch_and_and(p, v)
#define a_or(p, v)  __sync_fetch_and_or(p, v)

#define a_clz_32(x) __builtin_clz(x)
#define a_ctz_32(x) __builtin_ctz(x)

static inline void a_crash(void) {
    __builtin_trap();
}

#define LOCK(x)   ((void)0)
#define UNLOCK(x) ((void)0)
#define get_random_secret() ((uintptr_t)0xDECAFBADDEADBEEF)

// use macros to appropriately namespace these.
#define size_classes __malloc_size_classes
#define ctx __malloc_context
#define alloc_meta __malloc_alloc_meta
#define is_allzero __malloc_allzerop
#define dump_heap __dump_heap

#define malloc __libc_malloc_impl
#define realloc __libc_realloc
#define free __libc_free

#define USE_MADV_FREE 0

#if USE_REAL_ASSERT
#include <assert.h>
#else
#undef assert
#define assert(x) do { if (!(x)) a_crash(); } while(0)
#endif

#define brk(p) ((uintptr_t)__syscall1(SYS_brk, p))

#define mmap __mmap
#define munmap __munmap
#define madvise __madvise
#define mremap __mremap
#define mprotect __mprotect

static const int __malloc_replaced = 0;
static const int __aligned_alloc_replaced = 0;

#define DISABLE_ALIGNED_ALLOC (__malloc_replaced && !__aligned_alloc_replaced)

static inline void init_secret(uintptr_t *secret) {
    *secret = get_random_secret();
}

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#ifndef PAGESIZE
#define PAGESIZE PAGE_SIZE
#endif

#define MT 0

#define RDLOCK_IS_EXCLUSIVE 1

__attribute__((__visibility__("hidden")))
static int __malloc_lock[1];

#define LOCK_OBJ_DEF \
void __malloc_atfork(int who) { malloc_atfork(who); } \

static inline void rdlock()
{
	if (MT) LOCK(__malloc_lock);
}
static inline void wrlock()
{
	if (MT) LOCK(__malloc_lock);
}
static inline void unlock()
{
	UNLOCK(__malloc_lock);
}
static inline void upgradelock()
{
}
static inline void resetlock()
{
	__malloc_lock[0] = 0;
}

static inline void malloc_atfork(int who)
{
	if (who<0) rdlock();
	else if (who>0) resetlock();
	else unlock();
}

void *__libc_malloc_impl(size_t);
void *__libc_realloc_impl(void *, size_t);
void __libc_free_impl(void *);

#ifndef MADV_FREE
#define MADV_FREE 8
#endif

#endif
