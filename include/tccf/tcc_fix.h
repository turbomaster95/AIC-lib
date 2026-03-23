#ifndef TCC_FIX_H
#define TCC_FIX_H

#ifdef __TINYC__
    #include <bits/syscall.h>

    // 1. Architecture Mappings (Copy-pasted from your syscall.h)
    #define SYS_fork   __NR_clone
    #define SYS_open   __NR_openat
    #define SYS_openat __NR_openat
    #define SYS_creat  __NR_openat
    #define SYS_unlink __NR_unlinkat
    #define SYS_mkdir  __NR_mkdirat
    
    // Standard Mappings
    #define SYS_read          __NR_read
    #define SYS_write         __NR_write
    #define SYS_close         __NR_close
    #define SYS_wait4         __NR_wait4
    #define SYS_execve        __NR_execve
    #define SYS_getpid        __NR_getpid
    #define SYS_exit          __NR_exit
    #define SYS_ppoll         __NR_ppoll

    // 2. Redirect the generator to Clang's object
    extern long tcc_syscall_gen(long n, long a, long b, long c, long d, long e, long f);
    #define __syscall_gen tcc_syscall_gen

    // 3. Syscall Macros
    #define __syscall0(n)               __syscall_gen((long)n, 0, 0, 0, 0, 0, 0)
    #define __syscall1(n, a)            __syscall_gen((long)n, (long)a, 0, 0, 0, 0, 0)
    #define __syscall2(n, a, b)         __syscall_gen((long)n, (long)a, (long)b, 0, 0, 0, 0)
    #define __syscall3(n, a, b, c)      __syscall_gen((long)n, (long)a, (long)b, (long)c, 0, 0, 0)
    #define __syscall4(n, a, b, c, d)   __syscall_gen((long)n, (long)a, (long)b, (long)c, (long)d, 0, 0)
    #define __syscall5(n, a, b, c, d, e) __syscall_gen((long)n, (long)a, (long)b, (long)c, (long)d, (long)e, 0)
    #define __syscall6(n, a, b, c, d, e, f) __syscall_gen((long)n, (long)a, (long)b, (long)c, (long)d, (long)e, (long)f)

    // 4. BLOCK the real syscall.h from opening and crashing TCC
    #define AIC_INTERNAL_SYSCALL_H
#endif

#endif
