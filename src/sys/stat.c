#include <sys/stat.h>
#include <internal/syscall.h>
#include <errno.h>
#include <fcntl.h>

/* For AArch64, use newfstatat syscall */
#ifdef __aarch64__
#define SYS_newfstatat __NR_newfstatat
#else
#define SYS_newfstatat __NR_fstatat64
#endif

int stat(const char *pathname, struct stat *statbuf) {
    if (!pathname || !statbuf) {
        errno = EINVAL;
        return -1;
    }
    
#ifdef __aarch64__
    long ret = __syscall4(SYS_newfstatat, (long)AT_FDCWD, (long)pathname, (long)statbuf, 0);
#else
    long ret = __syscall4(SYS_newfstatat, (long)AT_FDCWD, (long)pathname, (long)statbuf, 0);
#endif
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int fstat(int fd, struct stat *statbuf) {
    if (!statbuf) {
        errno = EINVAL;
        return -1;
    }
    
    long ret = __syscall2(SYS_fstat, (long)fd, (long)statbuf);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int lstat(const char *pathname, struct stat *statbuf) {
    if (!pathname || !statbuf) {
        errno = EINVAL;
        return -1;
    }
    
    /* lstat is like stat but doesn't follow symlinks */
#ifdef __aarch64__
    long ret = __syscall4(SYS_newfstatat, (long)AT_FDCWD, (long)pathname, (long)statbuf, 0x100);  /* AT_SYMLINK_NOFOLLOW */
#else
    long ret = __syscall4(SYS_newfstatat, (long)AT_FDCWD, (long)pathname, (long)statbuf, 0x100);
#endif
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int access(const char *pathname, int mode) {
    if (!pathname) {
        errno = EINVAL;
        return -1;
    }
    
    /* Use faccessat with AT_FDCWD */
    long ret = __syscall3(SYS_faccessat, (long)AT_FDCWD, (long)pathname, (long)mode);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int chmod(const char *pathname, int mode) {
    if (!pathname) {
        errno = EINVAL;
        return -1;
    }
    
    long ret = __syscall3(SYS_fchmodat, (long)AT_FDCWD, (long)pathname, (long)mode);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int fchmod(int fd, int mode) {
    long ret = __syscall2(SYS_fchmod, (long)fd, (long)mode);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int mkdir(const char *pathname, int mode) {
    if (!pathname) {
        errno = EINVAL;
        return -1;
    }
    
#ifdef __aarch64__
    long ret = __syscall3(SYS_mkdirat, (long)AT_FDCWD, (long)pathname, (long)mode);
#else
    long ret = __syscall2(SYS_mkdir, (long)pathname, (long)mode);
#endif
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int rmdir(const char *pathname) {
    if (!pathname) {
        errno = EINVAL;
        return -1;
    }
    
#ifdef __aarch64__
    long ret = __syscall2(SYS_rmdir, (long)pathname, 0);
#else
    long ret = __syscall1(SYS_rmdir, (long)pathname);
#endif
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int unlink(const char *pathname) {
    if (!pathname) {
        errno = EINVAL;
        return -1;
    }
    
#ifdef __aarch64__
    long ret = __syscall3(SYS_unlinkat, (long)AT_FDCWD, (long)pathname, 0);
#else
    long ret = __syscall1(SYS_unlink, (long)pathname);
#endif
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}
