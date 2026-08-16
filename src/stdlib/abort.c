#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <internal/pal.h>

_Noreturn void abort(void) {
    sigset_t set;

    raise(SIGABRT);

    sigemptyset(&set);
    sigaddset(&set, SIGABRT);
    sigprocmask(SIG_UNBLOCK, &set, NULL);
    signal(SIGABRT, SIG_DFL);

    raise(SIGABRT);

#if defined(__GNUC__) || defined(__clang__)
    __builtin_trap();
#else
    pal_exit_group(127);
    pal_exit(127);
#endif

    for (;;) {}
}
