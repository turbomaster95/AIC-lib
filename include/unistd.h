#ifndef _UNISTD_H
#define _UNISTD_H

extern char **environ;

#include <stddef.h>
#include <bits/types.h>

/* File descriptor operations */
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
off_t lseek(int fd, off_t offset, int whence);

/* Pipe and dup */
int pipe(int pipefd[2]);
int pipe2(int pipefd[2], int flags);
int dup(int oldfd);
int dup2(int oldfd, int newfd);
int dup3(int oldfd, int newfd, int flags);

/* Process management */
pid_t fork(void);
pid_t getpid(void);
pid_t getppid(void);
int execve(const char *filename, char *const argv[], char *const envp[]);
int chdir(const char *path);
int fchdir(int fd);
char *getcwd(char *buf, size_t size);
pid_t waitpid(pid_t pid, int *status, int options);

/* User and group IDs */
uid_t getuid(void);
uid_t geteuid(void);
gid_t getgid(void);
gid_t getegid(void);
int setuid(uid_t uid);
int setgid(gid_t gid);
int setreuid(uid_t ruid, uid_t euid);
int setregid(gid_t rgid, gid_t egid);
int setpgid(pid_t pid, pid_t pgid);
pid_t getpgid(pid_t pid);
pid_t getpgrp(void);
pid_t setsid(void);
pid_t getsid(pid_t pid);

/* Process groups */
int setpgrp(void);

/* Standard file descriptors */
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

/* Seek constants */
#define SEEK_SET  0
#define SEEK_CUR  1
#define SEEK_END  2

/* Access constants */
#define F_OK    0
#define X_OK    1
#define W_OK    2
#define R_OK    4

/* sysconf constants */
#define _SC_ARG_MAX                 0
#define _SC_CHILD_MAX               5
#define _SC_CLK_TCK                 6
#define _SC_NGROUPS_MAX            10
#define _SC_OPEN_MAX               11
#define _SC_PAGESIZE               30
#define _SC_NPROCESSORS_CONF       64
#define _SC_NPROCESSORS_ONLN       65

/* Pathconf constants */
#define _PC_LINK_MAX                0
#define _PC_MAX_CANON              1
#define _PC_MAX_INPUT              2
#define _PC_NAME_MAX               3
#define _PC_PATH_MAX               4
#define _PC_PIPE_BUF               5

/* Function declarations */
long sysconf(int name);
long pathconf(const char *path, int name);
long fpathconf(int fd, int name);

/* Sleep functions */
unsigned int sleep(unsigned int seconds);
int usleep(unsigned long useconds);

/* Other */
int chown(const char *pathname, uid_t owner, gid_t group);
int lchown(const char *pathname, uid_t owner, gid_t group);
int link(const char *oldpath, const char *newpath);
int symlink(const char *target, const char *linkpath);
ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
int unlink(const char *pathname);
int rmdir(const char *pathname);
int access(const char *pathname, int mode);
int faccessat(int dirfd, const char *pathname, int mode, int flags);
int truncate(const char *path, off_t length);
int ftruncate(int fd, off_t length);
int sync(void);
int fsync(int fd);
int fdatasync(int fd);
char *ttyname(int fd);
int isatty(int fd);
int linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags);
int symlinkat(const char *target, int newdirfd, const char *linkpath);
ssize_t readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz);

#endif /* _UNISTD_H */
