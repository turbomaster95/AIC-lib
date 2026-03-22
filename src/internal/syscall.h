#ifndef AIC_INTERNAL_SYSCALL_H
#define AIC_INTERNAL_SYSCALL_H

#include <bits/syscall.h>

/* * 1. Architecture-Specific Alias Mapping
 * Modern architectures (AArch64) removed legacy syscalls like fork, 
 * open, and wait4 in favor of clone, openat, and wait4.
 */

// --- Process Management ---
#if defined(__aarch64__)
    #define SYS_fork   __NR_clone
    #define SYS_open   __NR_openat
    #define SYS_creat  __NR_openat
    #define SYS_unlink __NR_unlinkat
    #define SYS_mkdir  __NR_mkdirat
#else
    #define SYS_fork   __NR_fork
    #define SYS_open   __NR_open
    #define SYS_creat  __NR_creat
    #define SYS_unlink __NR_unlink
    #define SYS_mkdir  __NR_mkdir
#endif

// --- Standard Mappings ---
#define SYS_read      __NR_read
#define SYS_write     __NR_write
#define SYS_close     __NR_close
#define SYS_lseek     __NR_lseek
#define SYS_mmap      __NR_mmap
#define SYS_munmap    __NR_munmap
#define SYS_brk       __NR_brk
#define SYS_exit      __NR_exit
#define SYS_execve    __NR_execve
#define SYS_wait4     __NR_wait4
#define SYS_wait      __NR_wait4
#define SYS_getpid    __NR_getpid
#define SYS_getppid   __NR_getppid
#define SYS_chdir     __NR_chdir
#define SYS_getcwd    __NR_getcwd
#define SYS_kill      __NR_kill

/* * 2. Architecture-Specific Assembly Triggers
 */

#if defined(__x86_64__)
    /**
     * x86_64 ABI:
     * rax: syscall number
     * rdi, rsi, rdx, r10, r8, r9: arguments 1-6
     * Clobbers: rcx, r11 (used by the CPU for rip/rflags)
     */
    static inline long __syscall_gen(long n, long a, long b, long c, long d, long e, long f) {
        long ret;
        register long _r10 __asm__("r10") = d;
        register long _r8  __asm__("r8")  = e;
        register long _r9  __asm__("r9")  = f;
        __asm__ volatile (
            "syscall" 
            : "=a"(ret) 
            : "a"(n), "D"(a), "S"(b), "d"(c), "r"(_r10), "r"(_r8), "r"(_r9) 
            : "rcx", "r11", "memory"
        );
        return ret;
    }

#elif defined(__aarch64__)
    /**
     * AArch64 ABI:
     * x8: syscall number
     * x0 - x5: arguments 1-6
     */
    static inline long __syscall_gen(long n, long a, long b, long c, long d, long e, long f) {
        register long _x8 __asm__("x8") = n;
        register long _x0 __asm__("x0") = a;
        register long _x1 __asm__("x1") = b;
        register long _x2 __asm__("x2") = c;
        register long _x3 __asm__("x3") = d;
        register long _x4 __asm__("x4") = e;
        register long _x5 __asm__("x5") = f;
        __asm__ volatile (
            "svc 0" 
            : "+r"(_x0) 
            : "r"(_x8), "r"(_x1), "r"(_x2), "r"(_x3), "r"(_x4), "r"(_x5) 
            : "memory"
        );
        return _x0;
    }
#endif

/* * 3. Variadic Macro Wrappers 
 */
#define __syscall0(n)               __syscall_gen((long)n, 0, 0, 0, 0, 0, 0)
#define __syscall1(n, a)            __syscall_gen((long)n, (long)a, 0, 0, 0, 0, 0)
#define __syscall2(n, a, b)         __syscall_gen((long)n, (long)a, (long)b, 0, 0, 0, 0)
#define __syscall3(n, a, b, c)      __syscall_gen((long)n, (long)a, (long)b, (long)c, 0, 0, 0)
#define __syscall4(n, a, b, c, d)   __syscall_gen((long)n, (long)a, (long)b, (long)c, (long)d, 0, 0)
#define __syscall5(n, a, b, c, d, e) __syscall_gen((long)n, (long)a, (long)b, (long)c, (long)d, (long)e, 0)
#define __syscall6(n, a, b, c, d, e, f) __syscall_gen((long)n, (long)a, (long)b, (long)c, (long)d, (long)e, (long)f)

#endif /* AIC_INTERNAL_SYSCALL_H */
