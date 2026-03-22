#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

long waitpid(long pid, int *status, int options);

// Macros to parse the status word
#define WIFEXITED(s)   (((s) & 0x7f) == 0)
#define WEXITSTATUS(s) (((s) >> 8) & 0xff)
#define WTERMSIG(s)    ((s) & 0x7f)

#endif
