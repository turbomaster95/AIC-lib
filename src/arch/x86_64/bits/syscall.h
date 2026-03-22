#ifndef ARCH_SYSCALL_H
#define ARCH_SYSCALL_H

#define __NR_read    0
#define __NR_write   1
#define __NR_close   3
#define __NR_fstat   5
#define __NR_mmap    9
#define __NR_munmap  11
#define __NR_brk     12
#define __NR_rt_sigaction 13
#define __NR_rt_sigreturn 15
#define __NR_ioctl   16
#define __NR_fork    57   // x86_64 has a dedicated fork
#define __NR_execve  59
#define __NR_exit    60
#define __NR_wait4   61
#define __NR_clone   56   // Modern process creation

#endif
