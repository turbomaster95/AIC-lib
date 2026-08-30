#include <signal.h>
#include <string.h>
#include <errno.h>
#include <internal/pal.h>

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

int raise(int sig) {
    long pid = pal_getpid();
    return (int)pal_kill(pid, sig);
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    (void)how; (void)set; (void)oldset;
    return 0;
}

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
    (void)signum; (void)act; (void)oldact;
    return 0;
}

typedef void (*sighandler_t)(int);

sighandler_t signal(int signum, sighandler_t handler) {
    struct sigaction sa, old_sa;
    sa.sa_handler = handler;
    sa.sa_flags = 0;

    if (sigaction(signum, &sa, &old_sa) < 0) {
        return SIG_ERR;
    }
    return old_sa.sa_handler;
}
