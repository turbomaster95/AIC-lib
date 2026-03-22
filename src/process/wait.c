#include <internal/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t waitpid(pid_t pid, int *status, int options) {
    // wait4(pid, status_ptr, options, struct rusage *ru)
    // We pass 0 (NULL) for rusage if we don't need resource stats.
    return (pid_t)__syscall4(SYS_wait, (long)pid, (long)status, (long)options, 0);
}

// Helper macros to decode the status
int WIFEXITED(int status) {
    return (status & 0x7f) == 0;
}

int WEXITSTATUS(int status) {
    return (status >> 8) & 0xff;
}

int WTERMSIG(int status) {
    return status & 0x7f;
}
