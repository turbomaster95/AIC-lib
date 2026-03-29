#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifdef __x86_64_efi__
/* Example syscall-based stub. Adjust SYS_uptime and __syscall0
   to match your environment's syscall wrapper/header. */
#include <internal/syscall.h>
unsigned long uptime(void) {
    return (unsigned long)__syscall0(SYS_uptime);
}
#else
/* Read /proc/uptime and return integer seconds, 0 on error */
unsigned long uptime(void) {
    int fd = open("/proc/uptime", O_RDONLY | FD_CLOEXEC);
    if (fd < 0) return 0;

    char buf[64];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    int saved_errno = errno;
    close(fd);
    if (n <= 0) { errno = saved_errno; return 0; }
    buf[n] = '\0';

    unsigned long secs = 0;
    char *p = buf;
    while (*p == ' ' || *p == '\t') ++p;
    while (*p >= '0' && *p <= '9') {
        unsigned int d = (unsigned int)(*p - '0');
        if (secs > (unsigned long)((~(unsigned long)0 - d) / 10)) {
            secs = (unsigned long)(~(unsigned long)0);
            break;
        }
        secs = secs * 10 + d;
        ++p;
    }
    return secs;
}
#endif
