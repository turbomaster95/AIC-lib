#include <stdio.h>
#include <stdarg.h>

int sprintf(char *str, const char *format, ...) {
    va_list args;
    va_start(args, format);

    // Using -1 (or a very large number) for size since sprintf
    // doesn't check bounds, but vsnprintf needs a limit.
    int ret = vsnprintf(str, (size_t)-1, format, args);
    va_end(args);
    return ret;
}

