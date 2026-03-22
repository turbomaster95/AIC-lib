#include <internal/syscall.h>
#include <unistd.h>

long waitpid(long pid, int *status, int options) {
    return __syscall4(SYS_wait, pid, (long)status, options, 0);
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
