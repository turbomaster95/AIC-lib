#include <signal.h>
#include <unistd.h>
#include <internal/syscall.h>
#include <errno.h>

/* Simple signal handler table - for freestanding use */
static sighandler_t signal_handlers[_NSIG] = {0};

int sigemptyset(sigset_t *set) {
    if (!set) {
        errno = EINVAL;
        return -1;
    }
    *set = 0;
    return 0;
}

int sigfillset(sigset_t *set) {
    if (!set) {
        errno = EINVAL;
        return -1;
    }
    *set = ~0UL;
    return 0;
}

int sigaddset(sigset_t *set, int signum) {
    if (!set || signum <= 0 || signum >= _NSIG) {
        errno = EINVAL;
        return -1;
    }
    *set |= (1UL << (signum - 1));
    return 0;
}

int sigdelset(sigset_t *set, int signum) {
    if (!set || signum <= 0 || signum >= _NSIG) {
        errno = EINVAL;
        return -1;
    }
    *set &= ~(1UL << (signum - 1));
    return 0;
}

int sigismember(const sigset_t *set, int signum) {
    if (!set || signum <= 0 || signum >= _NSIG) {
        errno = EINVAL;
        return -1;
    }
    return (*set & (1UL << (signum - 1))) ? 1 : 0;
}

sighandler_t signal(int signum, sighandler_t handler) {
    struct sigaction act, oldact;
    
    if (signum <= 0 || signum >= _NSIG) {
        errno = EINVAL;
        return SIG_ERR;
    }
    
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    
    if (sigaction(signum, &act, &oldact) < 0) {
        return SIG_ERR;
    }
    
    return oldact.sa_handler;
}

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
    /* 
     * In a freestanding environment without kernel signal support,
     * we can only store the handler. Real signal handling requires
     * kernel support which may not be available.
     */
    if (signum <= 0 || signum >= _NSIG) {
        errno = EINVAL;
        return -1;
    }
    
    /* Store old action if requested */
    if (oldact) {
        oldact->sa_handler = signal_handlers[signum];
        oldact->sa_flags = 0;
        sigemptyset(&oldact->sa_mask);
    }
    
    /* Store new action if provided */
    if (act) {
        signal_handlers[signum] = act->sa_handler;
    }
    
    /* 
     * Note: This is a stub implementation. Real signal handling
     * requires the rt_sigaction syscall. For a full implementation:
     * 
     * #ifdef __aarch64__
     * long ret = __syscall4(__NR_rt_sigaction, signum, (long)act, (long)oldact, sizeof(sigset_t));
     * #else
     * long ret = __syscall4(__NR_rt_sigaction, signum, (long)act, (long)oldact, 8);
     * #endif
     */
    
    return 0;
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    /* Stub - real implementation needs kernel support */
    if (oldset) {
        *oldset = 0;
    }
    (void)how;
    (void)set;
    return 0;
}

int kill(pid_t pid, int sig) {
    if (pid < 0 || sig <= 0 || sig >= _NSIG) {
        errno = EINVAL;
        return -1;
    }
    
    long ret = __syscall2(SYS_kill, (long)pid, (long)sig);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int raise(int sig) {
    return kill(getpid(), sig);
}
