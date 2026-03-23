#ifndef ARCH_SYSCALL_H
#define ARCH_SYSCALL_H

/* x86_64 Linux syscall numbers */
#define __NR_read           0
#define __NR_write          1
#define __NR_open           2
#define __NR_close          3
#define __NR_stat           4
#define __NR_fstat          5
#define __NR_lseek          8
#define __NR_mmap           9
#define __NR_mprotect      10
#define __NR_munmap        11
#define __NR_brk           12
#define __NR_rt_sigaction  13
#define __NR_rt_sigreturn  15
#define __NR_ioctl         16
#define __NR_creat         85
#define __NR_fork          57
#define __NR_execve        59
#define __NR_exit          60
#define __NR_wait4         61
#define __NR_clone         56
#define __NR_unlink        87
#define __NR_mkdir         83
#define __NR_getcwd        79
#define __NR_chdir         80
#define __NR_getpid        39
#define __NR_getppid       110
#define __NR_kill          62
#define __NR_openat        257
#define __NR_unlinkat      263
#define __NR_mkdirat       258

#endif
