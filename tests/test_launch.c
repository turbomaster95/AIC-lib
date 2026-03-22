#define USING_STD_H_OK
#include <internal/syscall.h>
#include <std.h>

int main() {
    printf("AIC Launcher v2.0\n");

    long pid = __syscall1(SYS_fork, 17); // SIGCHLD = 17

    if (pid == 0) {
        char *binary = "/bin/ls";
        char *args[] = {binary, "-la", 0};
        char *env[] = {0};
        
        __syscall3(SYS_execve, (long)binary, (long)args, (long)env);
        
        // If we reach here, exec failed
        printf("Error: Could not execute binary.\n");
        exit(1);
    } else {
        int status;
        printf("Waiting for child %d...\n", pid);
        
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Child exited normally with code: %d\n", WEXITSTATUS(status));
        } else {
            printf("Child terminated by signal: %d\n", WTERMSIG(status));
        }
    }

    return 0;
}
