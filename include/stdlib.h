#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>

void *malloc(size_t size);
void free(void *ptr);
void exit(int code);
char *itoa(long val, char *buf, int base);
void *calloc(size_t nmemb, size_t size);
int atoi(const char *nptr);
_Noreturn void abort(void);

#define __libc_free free
#define __libc_malloc_impl malloc

#endif
