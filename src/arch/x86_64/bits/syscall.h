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
#define __NR_ppoll         7
#define __NR_fstatat64     262
#define __NR_faccessat     269
#define __NR_fchmodat      268
#define __NR_fchmod        91
#define __NR_rmdir         84
#define __NR_uname         63
#define __NR_select        23
#define __NR_pselect6      270
#define __NR_pipe          22
#define __NR_pipe2         293
#define __NR_dup           32
#define __NR_dup2          33
#define __NR_dup3          292
#define __NR_setpgid       154
#define __NR_getpgid       121
#define __NR_setsid        112
#define __NR_getsid        124
#define __NR_getpgrp       111  /* getpgrp is usually the same as getpgrp(0) libc wrapper */
#define __NR_getpid        39
#define __NR_getppid       110
#define __NR_setuid        105
#define __NR_setgid        106
#define __NR_getuid        102
#define __NR_getgid        104
#define __NR_geteuid       107
#define __NR_getegid       108
#define __NR_setreuid      113
#define __NR_setregid      114
#define __NR_gettimeofday  96
#define __NR_settimeofday  164
#define __NR_clock_gettime 228
#define __NR_clock_getres  229
#define __NR_nanosleep     35

#endif
