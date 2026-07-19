#include <stdio.h>
#include <stdint.h>

int snprintf(char *str, size_t size, const char *format, ...) {
    va_list ap;
    int result;

    va_start(ap, format);
    result = vsnprintf(str, size, format, ap);
    va_end(ap);

    return result;
}
