#ifndef MALLOC_GLUE_H
#define MALLOC_GLUE_H

#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>
//#include "libc.h"
//#include "lock.h"
//#include "dynlink.h"

#define a_cas(p, t, s) __sync_val_compare_and_swap(p, t, s)

#define a_store(p, v) __atomic_store_n(p, v, __ATOMIC_RELEASE)

#define a_and(p, v) __sync_fetch_and_and(p, v)
#define a_or(p, v)  __sync_fetch_and_or(p, v)

#define a_clz_32(x) __builtin_clz(x)
#define a_ctz_32(x) __builtin_ctz(x)

#define LOCK(x)   ((void)0)
#define UNLOCK(x) ((void)0)

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

#define brk(p) ((uintptr_t)__syscall(SYS_brk, p))

#define mmap __mmap
#define munmap __munmap
#define madvise __madvise
#define mremap __mremap
#define mprotect __mprotect

#define DISABLE_ALIGNED_ALLOC (__malloc_replaced && !__aligned_alloc_replaced)

static inline uint64_t get_random_secret()
{
	uint64_t secret = (uintptr_t)&secret * 1103515245;
	for (size_t i=0; libc.auxv[i]; i+=2)
		if (libc.auxv[i]==AT_RANDOM)
			memcpy(&secret, (char *)libc.auxv[i+1]+8, sizeof secret);
	return secret;
}

#ifndef PAGESIZE
#define PAGESIZE PAGE_SIZE
#endif

#define MT (libc.need_locks)

#define RDLOCK_IS_EXCLUSIVE 1

__attribute__((__visibility__("hidden")))
extern int __malloc_lock[1];

#define LOCK_OBJ_DEF \
void __malloc_atfork(int who) { malloc_atfork(who); } \
int __malloc_lock[1]

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

#endif
