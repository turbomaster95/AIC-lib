#include <internal/pal.h>
#include <internal/syscall.h>
#include <bits/syscall.h>
#include <sys/mman.h>

#define AT_FDCWD           -100

long pal_get_pagesize(void)   { return 4096; }
long pal_get_open_max(void)   { return 1024; }
long pal_get_nprocessors(void){ return 1; }

long pal_write(int fd, const void *buf, size_t count) {
    return __syscall3(SYS_write, fd, (long)buf, count);
}

long pal_read(int fd, void *buf, size_t count) {
    return __syscall3(SYS_read, fd, (long)buf, count);
}

long pal_open(const char *pathname, int flags, int mode) {
    return __syscall4(SYS_openat, AT_FDCWD, (long)pathname, flags, mode);
}

long pal_close(int fd) {
    return __syscall1(SYS_close, fd);
}

long pal_lseek(int fd, long offset, int whence) {
    return __syscall3(SYS_lseek, fd, offset, whence);
}

int pal_chdir(const char *path) {
    return (int)__syscall1(SYS_chdir, (long)path);
}

char *pal_getcwd(char *buf, size_t size) {
    long ret = __syscall2(SYS_getcwd, (long)buf, size);
    if (ret < 0) return (void *)0; 
    return buf;
}

int pal_pipe(int pipefd[2]) {
    return (int)__syscall2(SYS_pipe2, (long)pipefd, 0);
}

int pal_pipe2(int pipefd[2], int flags) {
    return (int)__syscall2(SYS_pipe2, (long)pipefd, flags);
}

int pal_dup(int oldfd) {
    return (int)__syscall1(SYS_dup, oldfd);
}

int pal_dup2(int oldfd, int newfd) {
    return (int)__syscall3(SYS_dup3, oldfd, newfd, 0);
}

int pal_dup3(int oldfd, int newfd, int flags) {
    return (int)__syscall3(SYS_dup3, oldfd, newfd, flags);
}

void *pal_mem_allocate(size_t size) {
    /* PROT_READ | PROT_WRITE = 0x3, MAP_PRIVATE | MAP_ANONYMOUS = 0x22 */
    long ret = __syscall6(SYS_mmap, 0, size, 0x3, 0x22, -1, 0);
    if (ret < 0 && ret >= -4095) return (void *)0;
    return (void *)ret;
}

int pal_mem_free(void *addr, size_t size) {
    return (int)__syscall2(SYS_munmap, (long)addr, size);
}

void pal_exit(int status) {
    __syscall1(SYS_exit, status);
    while (1); 
}

long pal_getpid(void)  { return __syscall0(SYS_getpid); }
long pal_getppid(void) { return __syscall0(SYS_getppid); }

pid_t pal_getpgid(pid_t pid) { return __syscall1(SYS_getpgid, pid); }
int  pal_setpgid(pid_t pid, pid_t pgid) { return (int)__syscall2(SYS_setpgid, pid, pgid); }

pid_t pal_setsid(void) { return (pid_t)__syscall0(SYS_setsid); }
pid_t pal_getsid(pid_t pid) { return (pid_t)__syscall1(SYS_getsid, pid); }

long pal_getuid(void)  { return __syscall0(SYS_getuid); }
long pal_geteuid(void) { return __syscall0(SYS_geteuid); }
long pal_getgid(void)  { return __syscall0(SYS_getgid); }
long pal_getegid(void) { return __syscall0(SYS_getegid); }

long pal_setuid(uid_t uid) { return __syscall1(SYS_setuid, uid); }
long pal_setgid(gid_t gid) { return __syscall1(SYS_setgid, gid); }
long pal_setreuid(uid_t ruid, uid_t euid) { return __syscall2(SYS_setreuid, ruid, euid); }
long pal_setregid(gid_t rgid, gid_t egid) { return __syscall2(SYS_setregid, rgid, egid); }

uintptr_t pal_brk(uintptr_t addr) {
    return (uintptr_t)__syscall1(SYS_brk, (long)addr);
}

void *pal_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    long ret = __syscall6(SYS_mmap, (long)addr, length, prot, flags, fd, offset);
    if (ret < 0 && ret >= -4095) return (void *)0;
    return (void *)ret;
}

int pal_munmap(void *addr, size_t length) {
    return (int)__syscall2(SYS_munmap, (long)addr, length);
}

int pal_mprotect(void *addr, size_t length, int prot) {
    return (int)__syscall3(SYS_mprotect, (long)addr, length, prot);
}

int pal_madvise(void *addr, size_t length, int advice) {
    return (int)__syscall3(SYS_madvise, (long)addr, length, advice);
}

void *pal_mremap(void *old_addr, size_t old_size, size_t new_size, int flags, void *new_addr) {
    return (void *)__syscall5(SYS_mremap, (long)old_addr, (long)old_size, (long)new_size, (long)flags, (long)new_addr);
}

long pal_kill(long pid, long sig) {
    return (long)__syscall2(SYS_kill, (long)pid, (long)sig);
}

/* --- File & I/O Additions --- */
int pal_openat(int dirfd, const char *pathname, int flags, mode_t mode) {
    return (int)__syscall4(SYS_openat, dirfd, (long)pathname, flags, mode);
}

int pal_fcntl(int fd, int cmd, long arg) {
    return (int)__syscall3(SYS_fcntl, fd, cmd, arg);
}

int pal_ioctl(int fd, unsigned long request, void *arg) {
    return (int)__syscall3(SYS_ioctl, fd, request, (long)arg);
}

/* --- Filesystem & Stat Additions --- */
int pal_fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags) {
    return (int)__syscall4(SYS_newfstatat, dirfd, (long)pathname, (long)statbuf, flags);
}

int pal_fstat(int fd, struct stat *statbuf) {
    return (int)__syscall2(SYS_fstat, fd, (long)statbuf);
}

int pal_faccessat(int dirfd, const char *pathname, int mode, int flags) {
    return (int)__syscall4(SYS_faccessat, dirfd, (long)pathname, mode, flags);
}

int pal_fchmodat(int dirfd, const char *pathname, mode_t mode, int flags) {
    return (int)__syscall4(SYS_fchmodat, dirfd, (long)pathname, mode, flags);
}

int pal_fchmod(int fd, mode_t mode) {
    return (int)__syscall2(SYS_fchmod, fd, mode);
}

int pal_mkdirat(int dirfd, const char *pathname, mode_t mode) {
    return (int)__syscall3(SYS_mkdirat, dirfd, (long)pathname, mode);
}

int pal_unlinkat(int dirfd, const char *pathname, int flags) {
    return (int)__syscall3(SYS_unlinkat, dirfd, (long)pathname, flags);
}

pid_t pal_fork(void) {
    return (pid_t)__syscall0(SYS_fork);
}

pid_t pal_wait4(pid_t pid, int *wstatus, int options, void *rusage) {
    return (pid_t)__syscall4(SYS_wait4, pid, (long)wstatus, options, (long)rusage);
}

int pal_execve(const char *filename, char *const argv[], char *const envp[]) {
    return (int)__syscall3(SYS_execve, (long)filename, (long)argv, (long)envp);
}

int pal_ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *tmo_p, const void *sigmask) {
    return (int)__syscall4(SYS_ppoll, (long)fds, nfds, (long)tmo_p, (long)sigmask);
}

int pal_pselect(int nfds, void *readfds, void *writefds, void *exceptfds, const struct timespec *timeout, const void *sigmask) {
    return (int)__syscall6(SYS_pselect6, nfds, (long)readfds, (long)writefds, (long)exceptfds, (long)timeout, (long)sigmask);
}

int pal_gettimeofday(struct timeval *tv, struct timezone *tz) {
    return (int)__syscall2(SYS_gettimeofday, (long)tv, (long)tz);
}

int pal_settimeofday(const struct timeval *tv, const struct timezone *tz) {
    return (int)__syscall2(SYS_settimeofday, (long)tv, (long)tz);
}

int pal_clock_gettime(clockid_t clock_id, struct timespec *tp) {
    return (int)__syscall2(SYS_clock_gettime, clock_id, (long)tp);
}

int pal_clock_getres(clockid_t clock_id, struct timespec *tp) {
    return (int)__syscall2(SYS_clock_getres, clock_id, (long)tp);
}

int pal_nanosleep(const struct timespec *req, struct timespec *rem) {
    return (int)__syscall2(SYS_nanosleep, (long)req, (long)rem);
}

int pal_uname(void *buf) {
    return (int)__syscall1(SYS_uname, (long)buf);
}
