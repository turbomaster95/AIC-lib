#define START "_start"
#define _dlstart_c _start_c

#include "../ld/dynlink.c"

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
void __dls2(unsigned char *base, size_t *sp)
{
    int argc = (int)sp[0];
    char **argv = (char **)(sp + 1);

    __libc_start_main(main, argc, argv, _init, _fini, 0);
}
