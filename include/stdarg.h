#ifndef _STDARG_H
#define _STDARG_H

typedef __builtin_va_list va_list;
#define va_start __builtin_va_start
#define va_arg __builtin_va_arg
#define va_copy __builtin_va_copy
#define va_end __builtin_va_end

/* Compatibility typedef for libio.h and other headers */
typedef va_list __gnuc_va_list;
#define _VA_LIST_DEFINED

#endif /* _STDARG_H */
