#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>
#include <stdarg.h>

int printf(const char *format, ...);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
int putchar(int c);
int getchar(void);
void flush(void);

// AIC Internal/Helper helpers
void print_str(const char *s);
void print_hex(unsigned long n);

#endif
