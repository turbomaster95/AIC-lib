#include <internal/syscall.h>
#include <stddef.h>

char **environ = NULL;
extern int main(int argc, char **argv, char **envp);

void _start_c(long *raw_stack) {
    int argc = (int)raw_stack[0];
    char **argv = (char **)&raw_stack[1];
    char **envp = (char **)&raw_stack[argc + 2];

    environ = envp;

    int status = main(argc, argv, envp);

   __syscall1(SYS_exit, status);
   for (;;);
}
