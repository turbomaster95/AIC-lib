#include <unistd.h>
#include <internal/syscall.h>
#include <errno.h>

int pipe(int pipefd[2]) {
    if (!pipefd) {
        errno = EFAULT;
        return -1;
    }
    
    long ret = __syscall2(SYS_pipe, (long)pipefd, 0);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int pipe2(int pipefd[2], int flags) {
    if (!pipefd) {
        errno = EFAULT;
        return -1;
    }
    
    long ret = __syscall2(SYS_pipe2, (long)pipefd, (long)flags);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int dup(int oldfd) {
    long ret = __syscall1(SYS_dup, (long)oldfd);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return (int)ret;
}

int dup2(int oldfd, int newfd) {
    long ret = __syscall2(SYS_dup2, (long)oldfd, (long)newfd);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return (int)ret;
}

int dup3(int oldfd, int newfd, int flags) {
    long ret = __syscall3(SYS_dup3, (long)oldfd, (long)newfd, (long)flags);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return (int)ret;
}
