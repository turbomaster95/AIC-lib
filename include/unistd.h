#ifndef _UNISTD_H
#define _UNISTD_H

extern char **environ;

#include <stddef.h>
#include <bits/types.h>

pid_t fork(void);
int execve(const char *filename, char *const argv[], char *const envp[]);
char *getcwd(char *buf, size_t size);

#endif
