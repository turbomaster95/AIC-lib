#include <stddef.h>
#include <internal/pal.h>

char **environ = NULL;
extern int main(int argc, char **argv, char **envp);

int __libc_start_main(
    int (*main_ptr)(int, char **, char **),
    int argc,
    char **argv,
    void (*init_func)(void),
    void (*fini_func)(void),
    void (*rtld_fini)(void),
    void *stack_end)
{
    if (init_func) {
        init_func();
    }

    char **envp = argv + argc + 1;

    environ = envp;

    int result = main_ptr(argc, argv, envp);

    if (fini_func) {
        fini_func();
    }

    pal_exit(result);
    return result;
}
