#include <stdio.h>

int vprintf(const char *format, va_list ap) {
    char buf[4096];
    int result = vsnprintf(buf, sizeof(buf), format, ap);
    print_str(buf);
    return result;
}
