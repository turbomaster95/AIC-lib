#include <fcntl.h>
#include <internal/syscall.h>
#include <errno.h>
#include <stdarg.h>

int open(const char *pathname, int flags, ...) {
    int mode = 0;
    
    /* If O_CREAT is specified, we need the mode argument */
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, int);
        va_end(ap);
    }
    
    /* On AArch64, open is actually openat with AT_FDCWD */
#ifdef __aarch64__
    long ret = __syscall4(SYS_openat, (long)AT_FDCWD, (long)pathname, (long)flags, (long)mode);
#else
    long ret = __syscall3(SYS_open, (long)pathname, (long)flags, (long)mode);
#endif
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return (int)ret;
}

int creat(const char *pathname, int mode) {
    /* creat() is equivalent to open() with O_CREAT|O_WRONLY|O_TRUNC */
    return open(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

int fcntl(int fd, int cmd, ...) {
    int arg;
    va_list ap;
    
    va_start(ap, cmd);
    arg = va_arg(ap, int);
    va_end(ap);
    
    /* For now, we only support basic fcntl operations */
    switch (cmd) {
        case F_DUPFD:
            /* Duplicate file descriptor - not fully implemented */
            errno = ENOSYS;
            return -1;
            
        case F_GETFD:
            /* Get file descriptor flags - return 0 (no flags set) */
            return 0;
            
        case F_SETFD:
            /* Set file descriptor flags - FD_CLOEXEC not supported */
            return 0;
            
        case F_GETFL:
            /* Get file status flags - not implemented */
            errno = ENOSYS;
            return -1;
            
        case F_SETFL:
            /* Set file status flags - O_NONBLOCK not supported */
            return 0;
            
        case F_GETLK:
        case F_SETLK:
        case F_SETLKW:
            /* File locking - not implemented */
            errno = ENOSYS;
            return -1;
            
        default:
            errno = EINVAL;
            return -1;
    }
}

int flock(int fd, int operation) {
    /* File locking - not implemented in freestanding mode */
    errno = ENOSYS;
    return -1;
}
