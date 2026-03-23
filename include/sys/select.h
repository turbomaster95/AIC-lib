#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H

#include <bits/types.h>
#include <time.h>

/* fd_set size */
#define FD_SETSIZE 1024
#define NFDBITS (8 * sizeof(unsigned long))

/* fd_set structure */
typedef struct {
    unsigned long fds_bits[FD_SETSIZE / NFDBITS];
} fd_set;

/* Macros for fd_set manipulation */
#define FD_ZERO(set) do { \
    for (int __i = 0; __i < sizeof(fd_set)/sizeof(unsigned long); __i++) \
        ((fd_set*)(set))->fds_bits[__i] = 0; \
} while (0)

#define FD_SET(fd, set) do { \
    ((fd_set*)(set))->fds_bits[(fd) / NFDBITS] |= (1UL << ((fd) % NFDBITS)); \
} while (0)

#define FD_CLR(fd, set) do { \
    ((fd_set*)(set))->fds_bits[(fd) / NFDBITS] &= ~(1UL << ((fd) % NFDBITS)); \
} while (0)

#define FD_ISSET(fd, set) \
    ((((fd_set*)(set))->fds_bits[(fd) / NFDBITS] & (1UL << ((fd) % NFDBITS))) != 0)

/* Function declarations */
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
int pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timespec *timeout, const sigset_t *sigmask);

#endif /* _SYS_SELECT_H */
