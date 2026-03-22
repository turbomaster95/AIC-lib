#ifndef _UNISTD_H
#define _UNISTD_H

#include <stddef.h>

long fork(void);
int execve(const char *filename, char *const argv[], char *const envp[]);
char *getcwd(char *buf, size_t size);

#endif
