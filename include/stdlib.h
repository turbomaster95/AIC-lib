#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>

void *malloc(size_t size);
void free(void *ptr);
void exit(int code);
char *itoa(long val, char *buf, int base);

#endif
