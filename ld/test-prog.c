/* test-prog.c - Simple program to test aic-ld.so */
/* Build: gcc -o test-prog test-prog.c -Wl,--dynamic-linker=./aic-ld.so */

/* We can't use stdio since we don't link libc properly yet.
 * Use raw syscalls instead. */

static inline long syscall3(long n, long a1, long a2, long a3) {
    long ret;
    register long r10 __asm__("r10") = a3;
    __asm__ volatile ("syscall" : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "r"(r10)
        : "rcx", "r11", "memory");
    return ret;
}

static inline void sys_write(int fd, const void *buf, long len) {
    syscall3(1, fd, (long)buf, len);
}

static inline void sys_exit(int code) {
    syscall3(60, code, 0, 0);
    __builtin_unreachable();
}

static int my_strlen(const char *s) {
    int n = 0;
    while (s[n]) n++;
    return n;
}

void _start(void) {
    const char *msg = "Hello from test-prog via aic-ld.so!\n";
    sys_write(1, msg, my_strlen(msg));
    sys_exit(0);
}
