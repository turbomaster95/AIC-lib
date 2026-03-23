#ifndef _POLL_H
#define _POLL_H

#include <bits/types.h>

/* Poll events */
#define POLLIN      0x0001
#define POLLPRI     0x0002
#define POLLOUT     0x0004
#define POLLERR     0x0008
#define POLLHUP     0x0010
#define POLLNVAL    0x0020
#define POLLRDNORM  0x0040
#define POLLRDBAND  0x0080
#define POLLWRNORM  0x0100
#define POLLWRBAND  0x0200
#define POLLMSG     0x0400
#define POLLREMOVE  0x1000
#define POLLRDHUP   0x2000

/* pollfd structure */
struct pollfd {
    int fd;
    short events;
    short revents;
};

/* Function declarations */
int poll(struct pollfd *fds, nfds_t nfds, int timeout);
int ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *tmo_p, const sigset_t *sigmask);

#endif /* _POLL_H */
