#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

int vdprintf(int fd, const char *format, va_list ap) {
    char *buf = NULL;
    int result;


    result = vasprintf(&buf, format, ap);
    if (result < 0) return -1;

    write(fd, buf, result);

    free(buf);
    return result;
}
