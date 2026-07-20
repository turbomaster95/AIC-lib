#define START "_start"
#include "crt_arch.h"

extern int main(int argc, char **argv, char **envp);
extern void _init(void) __attribute__((weak));
extern void _fini(void) __attribute__((weak));

extern int __libc_start_main(
    int (*main)(int, char **, char **),
    int argc,
    char **argv,
    void (*init)(void),
    void (*fini)(void),
    void (*ldso)(void)
);

__attribute__((__visibility__("hidden")))
void _start_c(long *p)
{
    int argc = (int)p[0];
    char **argv = (char **)(p + 1);

    // Hand off execution to libc startup routine
    __libc_start_main(main, argc, argv, _init, _fini, 0);
}
