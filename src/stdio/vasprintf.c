#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int vasprintf(char **strp, const char *format, va_list ap) {
    va_list ap_copy;
    int len;
    char *buf;

    if (!strp) return -1;

    /* Determine required length */
    va_copy(ap_copy, ap);
    len = vsnprintf(NULL, 0, format, ap_copy);
    va_end(ap_copy);

    if (len < 0) return -1;

    buf = (char *)malloc((size_t)len + 1);
    if (!buf) return -1;

    vsnprintf(buf, (size_t)len + 1, format, ap);
    *strp = buf;

    return len;
}
