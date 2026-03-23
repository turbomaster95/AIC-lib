#include <unistd.h>
#include <internal/syscall.h>
#include <errno.h>

pid_t getpgrp(void) {
    return (pid_t)__syscall0(SYS_getpgid);
}

pid_t getpgid(pid_t pid) {
    long ret = __syscall1(SYS_getpgid, (long)pid);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return (pid_t)ret;
}

int setpgid(pid_t pid, pid_t pgid) {
    long ret = __syscall2(SYS_setpgid, (long)pid, (long)pgid);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

pid_t setsid(void) {
    long ret = __syscall0(SYS_setsid);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return (pid_t)ret;
}

pid_t getsid(pid_t pid) {
    long ret = __syscall1(SYS_getsid, (long)pid);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return (pid_t)ret;
}
