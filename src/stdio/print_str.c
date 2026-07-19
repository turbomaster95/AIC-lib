#include <internal/pal.h>
#include <stdio.h>
#include <string.h>

void print_str(const char *s) {
    pal_write(1, (void*)s, strlen(s));
}
