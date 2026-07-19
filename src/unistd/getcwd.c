#include <unistd.h>
#include <errno.h>
#include <internal/pal.h>

char *getcwd(char *buf, size_t size) {
    char *ret = pal_getcwd(buf, size);
    if (!ret) {
        errno = ERANGE;
        return NULL;
    }
    return ret;
}
