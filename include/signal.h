#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <bits/types.h>
#include <stddef.h>

/* Signal numbers - Linux standard */
#define SIGHUP       1
#define SIGINT       2
#define SIGQUIT      3
#define SIGILL       4
#define SIGTRAP      5
#define SIGABRT      6
#define SIGIOT       SIGABRT
#define SIGBUS       7
#define SIGFPE       8
#define SIGKILL      9
#define SIGUSR1     10
#define SIGSEGV     11
#define SIGUSR2     12
#define SIGPIPE     13
#define SIGALRM     14
#define SIGTERM     15
#define SIGSTKFLT   16
#define SIGCHLD     17
#define SIGCONT     18
#define SIGSTOP     19
#define SIGTSTP     20
#define SIGTTIN     21
#define SIGTTOU     22
#define SIGURG      23
#define SIGXCPU     24
#define SIGXFSZ     25
#define SIGVTALRM   26
#define SIGPROF     27
#define SIGWINCH    28
#define SIGIO       29
#define SIGPOLL     SIGIO
#define SIGPWR      30
#define SIGSYS      31
#define SIGUNUSED   SIGSYS

/* Signal handling options */
#define SIG_BLOCK   0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

/* Signal actions */
#define SA_NOCLDSTOP 0x00000001
#define SA_NOCLDWAIT 0x00000002
#define SA_SIGINFO   0x00000004
#define SA_ONSTACK   0x08000000
#define SA_RESTART   0x10000000
#define SA_NODEFER   0x40000000
#define SA_RESETHAND 0x80000000

/* Signal set size */
#define _NSIG       64
#define NSIG        _NSIG

/* Special signal values */
#define SIG_ERR     ((__sighandler_t)-1)
#define SIG_DFL     ((__sighandler_t)0)
#define SIG_IGN     ((__sighandler_t)1)

/* Type definitions */
typedef unsigned long sigset_t;
typedef int sig_atomic_t;
typedef void (*__sighandler_t)(int);
typedef __sighandler_t sighandler_t;

/* Signal action structure */
struct sigaction {
    void (*sa_handler)(int);
    void (*sa_sigaction)(int, void *, void *);
    sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
};

/* siginfo_t for SA_SIGINFO */
typedef struct {
    int si_signo;
    int si_errno;
    int si_code;
    union {
        int _pad[128 - 2 * sizeof(int) - sizeof(long)];
    } _sifields;
} siginfo_t;

/* Function declarations */
sighandler_t signal(int signum, sighandler_t handler);
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
int sigaddset(sigset_t *set, int signum);
int sigdelset(sigset_t *set, int signum);
int sigismember(const sigset_t *set, int signum);
int kill(pid_t pid, int sig);
int raise(int sig);

/* BSD compatibility */
#define bsd_signal signal

#endif /* _SIGNAL_H */
