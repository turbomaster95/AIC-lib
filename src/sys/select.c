#include <sys/select.h>
#include <internal/pal.h>
#include <errno.h>
#include <stddef.h>

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
    if (nfds < 0 || nfds > FD_SETSIZE) {
        errno = EINVAL;
        return -1;
    }

    struct timespec ts;
    struct timespec *pts = NULL;

    if (timeout) {
        ts.tv_sec = timeout->tv_sec;
        ts.tv_nsec = timeout->tv_usec * 1000;
        pts = &ts;
    }

    int ret = pal_pselect(nfds, readfds, writefds, exceptfds, pts, NULL);

    if (ret < 0) {
        errno = -ret;
        return -1;
    }

    if (timeout && pts) {
        timeout->tv_sec = ts.tv_sec;
        timeout->tv_usec = ts.tv_nsec / 1000;
    }

    return ret;
}

int pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, 
            const struct timespec *timeout, const sigset_t *sigmask) {
    if (nfds < 0 || nfds > FD_SETSIZE) {
        errno = EINVAL;
        return -1;
    }

    int ret = pal_pselect(nfds, readfds, writefds, exceptfds, timeout, sigmask);

    if (ret < 0) {
        errno = -ret;
        return -1;
    }

    return ret;
}
