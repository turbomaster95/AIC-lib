#ifndef AIC_BITS_SYSCALL_H
#define AIC_BITS_SYSCALL_H

#define __NR_read  63
#define __NR_write 64
#define __NR_brk   214
#define __NR_exit  93
#define __NR_clone   220 // Modern fork
#define __NR_execve  221
#define __NR_wait4   260 // To wait for the child process
#define __NR_chdir 49
#define __NR_getcwd 17

#endif
