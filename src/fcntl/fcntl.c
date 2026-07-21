#include <fcntl.h>
#include <internal/pal.h>
#include <errno.h>
#include <stdarg.h>

int open(const char *pathname, int flags, ...) {
    mode_t mode = 0;

    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }

    int ret = pal_open(pathname, flags, mode);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }

    return ret;
}

int creat(const char *pathname, int mode) {
    return open(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

int fcntl(int fd, int cmd, ...) {
    void *arg = NULL;
    va_list ap;

    va_start(ap, cmd);
    arg = va_arg(ap, void *);
    va_end(ap);

    int ret = pal_fcntl(fd, cmd, (long)arg);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }

    return ret;
}

int flock(int fd, int operation) {
    (void)fd;
    (void)operation;
    errno = ENOSYS;
    return -1;
}
