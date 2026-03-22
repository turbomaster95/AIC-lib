#include <stdio.h>
#include <stdarg.h>

int printf(const char *format, ...) {
    char buffer[1024]; // Simple stack buffer for now
    va_list ap;
    va_start(ap, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, ap);
    va_end(ap);
    
    print_str(buffer);
    return len;
}
