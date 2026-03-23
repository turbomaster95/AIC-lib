#include <internal/syscall.h>
#include <unistd.h>
#include <errno.h>

ssize_t read(int fd, void *buf, size_t count) {
    long ret = __syscall3(SYS_read, (long)fd, (long)buf, (long)count);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (ssize_t)ret;
}

ssize_t write(int fd, const void *buf, size_t count) {
    long ret = __syscall3(SYS_write, (long)fd, (long)buf, (long)count);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (ssize_t)ret;
}

int close(int fd) {
    long ret = __syscall1(SYS_close, (long)fd);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return 0;
}

off_t lseek(int fd, off_t offset, int whence) {
    long ret = __syscall3(SYS_lseek, (long)fd, (long)offset, (long)whence);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return (off_t)ret;
}

pid_t getpid(void) {
    return (pid_t)__syscall0(SYS_getpid);
}

pid_t getppid(void) {
    return (pid_t)__syscall0(SYS_getppid);
}
