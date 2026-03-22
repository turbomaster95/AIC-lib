#include <unistd.h>
#include <internal/syscall.h>

char *getcwd(char *buf, size_t size) {
    long ret = __syscall2(SYS_getcwd, (long)buf, size);
    if (ret < 0) return NULL;
    return buf;
}
