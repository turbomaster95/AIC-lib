#include <sys/select.h>
#include <internal/syscall.h>
#include <errno.h>

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
    if (nfds < 0 || nfds > FD_SETSIZE) {
        errno = EINVAL;
        return -1;
    }
    
    long ret = __syscall5(SYS_select, (long)nfds, (long)readfds, (long)writefds, (long)exceptfds, (long)timeout);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return (int)ret;
}

int pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timespec *timeout, const sigset_t *sigmask) {
    if (nfds < 0 || nfds > FD_SETSIZE) {
        errno = EINVAL;
        return -1;
    }
    
    long ret = __syscall6(SYS_pselect6, (long)nfds, (long)readfds, (long)writefds, (long)exceptfds, (long)timeout, (long)sigmask);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return (int)ret;
}
