#include <internal/syscall.h>
#include <unistd.h>

long fork(void) {
#if defined(__aarch64__)
    // clone(flags, stack, parent_tid, child_tid, tls)
    // 17 is SIGCHLD, which tells the kernel to notify the parent when child dies.
    return __syscall5(SYS_fork, 17, 0, 0, 0, 0);
#else
    return __syscall0(SYS_fork);
#endif
}
