#include <stdio.h>
#include <stdarg.h>

int asprintf(char **strp, const char *format, ...) {
    va_list ap;
    int result;

    va_start(ap, format);
    result = vasprintf(strp, format, ap);
    va_end(ap);

    return result;
}
