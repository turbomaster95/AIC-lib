#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <internal/pal.h>
#include <unistd.h>
#include <sys/stat.h>

char *getenv(const char *name) {
    if (!name || !environ) return NULL;
    size_t len = strlen(name);
    for (char **env = environ; *env != NULL; env++) {
        if (strncmp(*env, name, len) == 0 && (*env)[len] == '=') {
            return *env + len + 1;
        }
    }
    return NULL;
}

int system(const char *command) {
    if (!command) return 1;
    errno = ENOSYS;
    return -1;
}

int remove(const char *pathname) {
    return pal_unlinkat(AT_FDCWD, pathname, 0);
}

int rename(const char *oldpath, const char *newpath) {
    (void)oldpath; (void)newpath;
    errno = ENOSYS;
    return -1;
}

int mkstemp(char *template) {
    (void)template;
    errno = ENOSYS;
    return -1;
}

int isatty(int fd) {
    struct stat st;
    if (pal_fstat(fd, &st) < 0) {
        return 0;
    }
    return pal_ioctl(fd, 0x5401 /* TCGETS */, &st) == 0;
}
