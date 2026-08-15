/* src/signal/signal.c */
#include <signal.h>
#include <string.h>
#include <errno.h>

int sigemptyset(sigset_t *set) {
    if (!set) {
        errno = EINVAL;
        return -1;
    }
    memset(set, 0, sizeof(sigset_t));
    return 0;
}

int sigfillset(sigset_t *set) {
    if (!set) {
        errno = EINVAL;
        return -1;
    }
    memset(set, 0xff, sizeof(sigset_t));
    return 0;
}

int sigaddset(sigset_t *set, int signum) {
    if (!set || signum <= 0 || signum > 64) {
        errno = EINVAL;
        return -1;
    }
    unsigned long s = signum - 1;
    set->__bits[s / (8 * sizeof(long))] |= 1UL << (s % (8 * sizeof(long)));
    return 0;
}

int sigdelset(sigset_t *set, int signum) {
    if (!set || signum <= 0 || signum > 64) {
        errno = EINVAL;
        return -1;
    }
    unsigned long s = signum - 1;
    set->__bits[s / (8 * sizeof(long))] &= ~(1UL << (s % (8 * sizeof(long))));
    return 0;
}

int sigismember(const sigset_t *set, int signum) {
    if (!set || signum <= 0 || signum > 64) {
        errno = EINVAL;
        return -1;
    }
    unsigned long s = signum - 1;
    return !!(set->__bits[s / (8 * sizeof(long))] & (1UL << (s % (8 * sizeof(long)))));
}
