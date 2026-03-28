#include <unistd.h>
#include <internal/syscall.h>

unsigned long uptime(void) {
    return (unsigned long)__syscall0(SYS_uptime);
}
