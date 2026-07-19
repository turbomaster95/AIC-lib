#include <stdio.h>
#include <stdarg.h>

int dprintf(int fd, const char *format, ...) {
    va_list ap;
    int result;

    va_start(ap, format);
    result = vdprintf(fd, format, ap);
    va_end(ap);

    return result;
}
