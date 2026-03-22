#include <internal/syscall.h>
#include <unistd.h>

int execve(const char *filename, char *const argv[], char *const envp[]) {
    // execve is syscall #3 in our generic mapper
    return __syscall3(SYS_execve, (long)filename, (long)argv, (long)envp);
}
