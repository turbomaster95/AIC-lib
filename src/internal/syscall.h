#ifndef AIC_INTERNAL_SYSCALL_H
#define AIC_INTERNAL_SYSCALL_H

#include <bits/syscall.h>

/* Alias the architecture-specific Kernel numbers to Generic names */
#define SYS_read  __NR_read
#define SYS_write __NR_write
#define SYS_brk   __NR_brk
#define SYS_exit  __NR_exit
#if defined(__aarch64__)
    #define SYS_fork __NR_clone
#else
    #define SYS_fork __NR_fork
#endif
#define SYS_execve __NR_execve
#define SYS_wait   __NR_wait4
#define SYS_chdir  __NR_chdir
#define SYS_getcwd __NR_getcwd

/* Architecture-specific Assembly Triggers */
#if defined(__x86_64__)
    static inline long __syscall_gen(long n, long a, long b, long c, long d, long e, long f) {
        long ret;
        register long r10 __asm__("r10") = d;
        register long r8  __asm__("r8")  = e;
        register long r9  __asm__("r9")  = f;
        __asm__ volatile ("syscall" : "=a"(ret) : "a"(n), "D"(a), "S"(b), "d"(c), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
        return ret;
    }
#elif defined(__aarch64__)
    static inline long __syscall_gen(long n, long a, long b, long c, long d, long e, long f) {
        register long _x8 __asm__("x8") = n;
        register long _x0 __asm__("x0") = a;
        register long _x1 __asm__("x1") = b;
        register long _x2 __asm__("x2") = c;
        register long _x3 __asm__("x3") = d;
        register long _x4 __asm__("x4") = e;
        register long _x5 __asm__("x5") = f;
        __asm__ volatile ("svc 0" : "+r"(_x0) : "r"(_x8), "r"(_x1), "r"(_x2), "r"(_x3), "r"(_x4), "r"(_x5) : "memory");
        return _x0;
    }
#endif
#define __syscall0(n)               __syscall_gen(n, 0, 0, 0, 0, 0, 0)
#define __syscall1(n, a)            __syscall_gen(n, (long)a, 0, 0, 0, 0, 0)
#define __syscall2(n, a, b)         __syscall_gen(n, (long)a, (long)b, 0, 0, 0, 0)
#define __syscall3(n, a, b, c)      __syscall_gen(n, (long)a, (long)b, (long)c, 0, 0, 0)
#define __syscall4(n, a, b, c, d)   __syscall_gen(n, (long)a, (long)b, (long)c, (long)d, 0, 0)
#define __syscall5(n, a, b, c, d, e) __syscall_gen(n, (long)a, (long)b, (long)c, (long)d, (long)e, 0)
#endif

