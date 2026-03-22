#include <internal/syscall.h>
#include <unistd.h>

int chdir(const char *path) {
    return (int)__syscall1(SYS_chdir, (long)path);
}
