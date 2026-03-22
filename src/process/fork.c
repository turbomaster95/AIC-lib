#include <internal/syscall.h>
#include <unistd.h>

pid_t fork(void) {
#ifdef __aarch64__
    // AArch64: clone(SIGCHLD, 0, 0, 0, 0)
    return (pid_t)__syscall5(SYS_fork, 17, 0, 0, 0, 0);
#else
    // x86_64: fork() has 0 arguments
    return (pid_t)__syscall0(SYS_fork);
#endif
}
