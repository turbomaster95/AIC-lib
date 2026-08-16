#ifndef WCHAR_H
#define WCHAR_H

#include <features.h>

#define __NEED_FILE
#define __NEED___isoc_va_list
#define __NEED_size_t
#define __NEED_wchar_t
#define __NEED_wint_t
#define __NEED_mbstate_t

#if __USE_ISOC11
 #define __NEED_struct__IO_FILE
#endif

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(__USE_XOPEN) || defined(__USE_GNU) || defined(__USE_BSD)
 #define __NEED_locale_t
 #define __NEED_va_list
#endif

#if defined(__USE_XOPEN) || defined(__USE_GNU) || defined(__USE_BSD)
 #define __NEED_wctype_t
#endif

#include <bits/alltypes.h>

#if L'\0'-1 > 0
 #define WCHAR_MAX (0xffffffffu+L'\0')
 #define WCHAR_MIN (0+L'\0')
#else
 #define WCHAR_MAX (0x7fffffff+L'\0')
 #define WCHAR_MIN (-1-0x7fffffff+L'\0')
#endif

#ifndef NULL
 #define NULL ((void*)0)
#endif

#undef WEOF
#define WEOF 0xffffffffU

#endif
