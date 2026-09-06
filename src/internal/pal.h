#ifndef _AIC_PAL_H
#define _AIC_PAL_H

#include <stddef.h>
#include <stdint.h>

#define __NEED_size_t
#define __NEED_pid_t
#define __NEED_gid_t
#define __NEED_off_t
#define __NEED_uid_t
#define __NEED_mode_t
#define __NEED_clockid_t
#define __NEED_nfds_t
#define __NEED_time_t
#define __NEED_suseconds_t
#define __NEED_struct_timeval
#include <bits/alltypes.h>

struct stat;
struct timezone;
struct timespec;
struct pollfd;

// Sysconf
long pal_get_pagesize(void);
long pal_get_open_max(void);
long pal_get_nprocessors(void);

// I/O Operations
long pal_write(int fd, const void *buf, size_t count);
long pal_read(int fd, void *buf, size_t count);
long pal_open(const char *pathname, int flags, int mode);
long pal_close(int fd);
long pal_lseek(int fd, long offset, int whence);
int pal_openat(int dirfd, const char *pathname, int flags, mode_t mode);
int pal_fcntl(int fd, int cmd, long arg);
int pal_ioctl(int fd, unsigned long request, void *arg);

// Directory Operations
int   pal_chdir(const char *path);
char *pal_getcwd(char *buf, size_t size);
int pal_fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags);
int pal_fstat(int fd, struct stat *statbuf);
int pal_faccessat(int dirfd, const char *pathname, int mode, int flags);
int pal_fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);
int pal_fchmod(int fd, mode_t mode);
int pal_mkdirat(int dirfd, const char *pathname, mode_t mode);
int pal_unlinkat(int dirfd, const char *pathname, int flags);
long pal_getdents64(int dirfd, char *buf, size_t size);

// Misc
int pal_pipe(int pipefd[2]);
int pal_pipe2(int pipefd[2], int flags);
int pal_dup(int oldfd);
int pal_dup2(int oldfd, int newfd);
int pal_dup3(int oldfd, int newfd, int flags);
int pal_uname(void *buf);

// Time operations
int pal_gettimeofday(struct timeval *tv, struct timezone *tz);
int pal_settimeofday(const struct timeval *tv, const struct timezone *tz);
int pal_clock_gettime(clockid_t clock_id, struct timespec *tp);
int pal_clock_getres(clockid_t clock_id, struct timespec *tp);
int pal_nanosleep(const struct timespec *req, struct timespec *rem);

// Memory Operations
void *pal_mem_allocate(size_t size);
int   pal_mem_free(void *addr, size_t size);
uintptr_t pal_brk(uintptr_t addr);
void     *pal_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int       pal_munmap(void *addr, size_t length);
int       pal_mprotect(void *addr, size_t length, int prot);
int       pal_madvise(void *addr, size_t length, int advice);
void *pal_mremap(void *old_addr, size_t old_size, size_t new_size, int flags, void *new_addr);

// Process Control
void pal_exit(int status) __attribute__((__noreturn__));
int pal_ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *tmo_p, const void *sigmask);
int pal_pselect(int nfds, void *readfds, void *writefds, void *exceptfds, const struct timespec *timeout, const void *sigmask);
pid_t pal_getpgid(pid_t pid);
int   pal_setpgid(pid_t pid, pid_t pgid);
pid_t pal_setsid(void);
pid_t pal_getsid(pid_t pid);
long pal_setuid(uid_t uid);
long pal_setgid(gid_t gid);
long pal_setreuid(uid_t ruid, uid_t euid);
long pal_setregid(gid_t rgid, gid_t egid);
long pal_getuid(void);
long pal_geteuid(void);
long pal_getgid(void);
long pal_getegid(void);
long pal_getpid(void);
long pal_getppid(void);
long pal_kill(long pid, long sig);
pid_t pal_fork(void);
pid_t pal_wait4(pid_t pid, int *wstatus, int options, void *rusage);
int   pal_execve(const char *filename, char *const argv[], char *const envp[]);
int  pal_sched_yield(void);

#endif

