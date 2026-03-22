#include <stdio.h>
#include <stdarg.h> // We can use the compiler's stdarg.h
#include <stdlib.h>

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
    size_t i = 0;
    const char *p = format;
    char buf[64];

    while (*p && i < size - 1) {
        if (*p == '%') {
            p++;
            if (*p == 'd') {
                itoa(va_arg(ap, int), buf, 10);
                for (int j = 0; buf[j] && i < size - 1; j++) str[i++] = buf[j];
            } else if (*p == 'x' || *p == 'p') {
                itoa(va_arg(ap, unsigned long), buf, 16);
                for (int j = 0; buf[j] && i < size - 1; j++) str[i++] = buf[j];
            } else if (*p == 's') {
                char *s = va_arg(ap, char *);
                for (int j = 0; s[j] && i < size - 1; j++) str[i++] = s[j];
            } else if (*p == 'c') {
                str[i++] = (char)va_arg(ap, int);
            }
        } else {
            str[i++] = *p;
        }
        p++;
    }
    str[i] = '\0';
    return i;
}
