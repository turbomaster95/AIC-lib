#include <unistd.h>
#include <errno.h>
#include <internal/pal.h>

ssize_t read(int fd, void *buf, size_t count) {
    long ret = pal_read(fd, buf, count);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (ssize_t)ret;
}

ssize_t write(int fd, const void *buf, size_t count) {
    long ret = pal_write(fd, buf, count);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (ssize_t)ret;
}

int close(int fd) {
    long ret = pal_close(fd);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return 0;
}

off_t lseek(int fd, off_t offset, int whence) {
    long ret = pal_lseek(fd, offset, whence);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (off_t)ret;
}

pid_t getpid(void) {
    return (pid_t)pal_getpid();
}

pid_t getppid(void) {
    return (pid_t)pal_getppid();
}

