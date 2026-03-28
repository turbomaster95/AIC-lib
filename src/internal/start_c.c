#include <internal/syscall.h> // Ensure this includes the arch-specific bits
#include <stddef.h>

char **environ = NULL;
extern int main(int argc, char **argv, char **envp);

void _start_c(long *raw_stack) {
    int argc = (int)raw_stack[0];
    char **argv = (char **)&raw_stack[1];
    char **envp = (char **)&raw_stack[argc + 2];

    environ = envp;

    int status = main(argc, argv, envp);

    // Use a generic exit call if you've implemented it, 
    // or use the arch-specific inline:
// #ifdef __aarch64__
   // __asm__ volatile ("mov x0, %0\n mov x8, #93\n svc #0" : : "r"((long)status) : "x0", "x8");
// #else
   //  __asm__ volatile ("mov %0, %%rdi\n mov $60, %%rax\n syscall" : : "r"((long)status) : "rdi", "rax");
// #endif
   __syscall1(SYS_exit, status);
   for (;;);
}
