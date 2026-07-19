#include <unistd.h>
#include <internal/pal.h>
#include <errno.h>

pid_t getpgrp(void) {
    long ret = pal_getpgid(0);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return (pid_t)ret;
}

pid_t getpgid(pid_t pid) {
    long ret = pal_getpgid(pid);

    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }

    return (pid_t)ret;
}

int setpgid(pid_t pid, pid_t pgid) {
    long ret = pal_setpgid(pid, pgid);

    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }

    return 0;
}

pid_t setsid(void) {
    long ret = pal_setsid();

    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }

    return (pid_t)ret;
}

pid_t getsid(pid_t pid) {
    long ret = pal_getsid(pid);

    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }

    return (pid_t)ret;
}

