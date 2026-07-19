#ifndef _AIC_PAL_H
#define _AIC_PAL_H

#include <stddef.h>
#include <stdint.h>
#include <bits/types.h>

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

// Directory Operations
int   pal_chdir(const char *path);
char *pal_getcwd(char *buf, size_t size);

// Misc
int pal_pipe(int pipefd[2]);
int pal_pipe2(int pipefd[2], int flags);
int pal_dup(int oldfd);
int pal_dup2(int oldfd, int newfd);
int pal_dup3(int oldfd, int newfd, int flags);

// Memory Operations
void *pal_mem_allocate(size_t size);
int   pal_mem_free(void *addr, size_t size);
uintptr_t pal_brk(uintptr_t addr);
void     *pal_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int       pal_munmap(void *addr, size_t length);
int       pal_mprotect(void *addr, size_t length, int prot);
int       pal_madvise(void *addr, size_t length, int advice);

// Process Control
void pal_exit(int status) __attribute__((__noreturn__));
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

#endif

