#include <sys/ioctl.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <internal/pal.h>

int ioctl(int fd, unsigned long request, ...) {
    void *arg;
    va_list ap;
    va_start(ap, request);
    arg = va_arg(ap, void *);
    va_end(ap);

    int ret = pal_ioctl(fd, request, arg);

    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }

    return (int)ret;
}
