#include <poll.h>
#include <sys/select.h>
#include <internal/syscall.h>
#include <errno.h>
#include <string.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    if (!fds && nfds > 0) {
        errno = EFAULT;
        return -1;
    }
    
    long ret = __syscall3(SYS_ppoll, (long)fds, (long)nfds, (long)NULL);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return (int)ret;
}

int ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *tmo_p, const sigset_t *sigmask) {
    if (!fds && nfds > 0) {
        errno = EFAULT;
        return -1;
    }
    
    long ret = __syscall3(SYS_ppoll, (long)fds, (long)nfds, (long)tmo_p);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return (int)ret;
}
