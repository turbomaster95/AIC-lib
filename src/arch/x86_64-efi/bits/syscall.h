#ifndef ARCH_SYSCALL_H
#define ARCH_SYSCALL_H

/* Bios-Nim OS syscall numbers for x86_64-efi */
#define __NR_read           0    /* Not yet implemented meaningfully */
#define __NR_write          1    /* Maps to kernel puts */
#define __NR_getc           3
#define __NR_put_char       4
#define __NR_exit           60
#define __NR_uptime         61
#define __NR_uname          62
#define __NR_read_disk      63
#define __NR_ls             64
#define __NR_cat            65
#define __NR_mkdir          66
#define __NR_touch          67
#define __NR_write_file     68
#define __NR_clear          69
#define __NR_execve         70   /* Maps to kernel exec */

/* Common dummy aliases for AIC - mapped to -1 (unsupported) */
#define __NR_open           -1
#define __NR_openat         -1
#define __NR_close          -1
#define __NR_stat           -1
#define __NR_fstat          -1
#define __NR_lstat          -1
#define __NR_newfstatat     -1
#define __NR_lseek          -1
#define __NR_mmap           -1
#define __NR_munmap         -1
#define __NR_mprotect       -1
#define __NR_brk            -1
#define __NR_poll           -1
#define __NR_ppoll          -1
#define __NR_select         -1
#define __NR_pselect6       -1
#define __NR_rt_sigaction   -1
#define __NR_rt_sigreturn   -1
#define __NR_ioctl          -1
#define __NR_gettimeofday   -1
#define __NR_settimeofday   -1
#define __NR_clock_gettime  -1
#define __NR_clock_getres   -1
#define __NR_nanosleep      -1
#define __NR_fork           -1
#define __NR_clone          -1
#define __NR_wait4          -1
#define __NR_kill           -1
#define __NR_chdir          -1
#define __NR_fchdir         -1
#define __NR_getcwd         -1
#define __NR_dup            -1
#define __NR_dup2           -1
#define __NR_dup3           -1
#define __NR_pipe           -1
#define __NR_pipe2          -1
#define __NR_getuid         -1
#define __NR_geteuid        -1
#define __NR_getgid         -1
#define __NR_getegid        -1
#define __NR_setuid         -1
#define __NR_setgid         -1
#define __NR_setreuid       -1
#define __NR_setregid       -1
#define __NR_setpgid        -1
#define __NR_getpgid        -1
#define __NR_getpgrp        -1
#define __NR_setsid         -1
#define __NR_getsid         -1
#define __NR_fsync          -1
#define __NR_fdatasync      -1
#define __NR_time           -1
#define __NR_unlink         -1
#define __NR_unlinkat       -1
#define __NR_mkdirat        -1
#define __NR_fchmodat       -1
#define __NR_fchmod         -1
#define __NR_fchownat       -1
#define __NR_fchown         -1
#define __NR_chown          -1
#define __NR_rmdir          -1
#define __NR_faccessat      -1
#define __NR_creat          -1
#define __NR_getpid         -1
#define __NR_getppid        -1
#define __NR_fstatat64      -1
#define __NR_sysconf        -1

#endif
