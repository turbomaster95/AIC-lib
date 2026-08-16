#include <features.h>

#undef assert

#ifdef NDEBUG
#define assert(ignore) ((void)0)
#else
#if defined(__USE_ISOC99)
#define assert(expression) \
    ((expression) ? (void)0 : __assert_fail(#expression, __FILE__, __LINE__, __func__))
#else
#define assert(expression) \
    ((expression) ? (void)0 : __assert_fail(#expression, __FILE__, __LINE__, (const char *)0))
#endif
#endif

#ifndef _ASSERT_H
#define _ASSERT_H

_Noreturn void __assert_fail(const char *expr, const char *file, int line, const char *func);

#endif /* _ASSERT_H */
