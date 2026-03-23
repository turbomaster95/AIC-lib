#include <termios.h>
#include <internal/syscall.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>

int ioctl(int fd, unsigned long request, ...) {
    void *arg;
    va_list ap;
    
    va_start(ap, request);
    arg = va_arg(ap, void *);
    va_end(ap);
    
    long ret = __syscall3(SYS_ioctl, (long)fd, (long)request, (long)arg);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return (int)ret;
}

int tcgetattr(int fd, struct termios *termios_p) {
    if (!termios_p) {
        errno = EINVAL;
        return -1;
    }
    
    long ret = __syscall2(SYS_ioctl, (long)fd, TCGETS, (long)termios_p);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int tcsetattr(int fd, int optional_actions, const struct termios *termios_p) {
    unsigned long request;
    
    if (!termios_p) {
        errno = EINVAL;
        return -1;
    }
    
    switch (optional_actions) {
        case TCSANOW:
            request = TCSETS;
            break;
        case TCSADRAIN:
            request = TCSETSW;
            break;
        case TCSAFLUSH:
            request = TCSETSF;
            break;
        default:
            errno = EINVAL;
            return -1;
    }
    
    long ret = __syscall2(SYS_ioctl, (long)fd, request, (long)termios_p);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int tcflush(int fd, int queue_selector) {
    long ret = __syscall2(SYS_ioctl, (long)fd, TCFLSH, (long)queue_selector);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int tcflow(int fd, int action) {
    long ret = __syscall2(SYS_ioctl, (long)fd, TCXONC, (long)action);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int tcsendbreak(int fd, int duration) {
    long ret = __syscall2(SYS_ioctl, (long)fd, TCSBRK, (long)0);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

int tcdrain(int fd) {
    long ret = __syscall2(SYS_ioctl, (long)fd, TCSBRK, (long)1);
    
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    
    return 0;
}

speed_t cfgetispeed(const struct termios *termios_p) {
    if (!termios_p) {
        errno = EINVAL;
        return (speed_t)-1;
    }
    return (speed_t)(termios_p->c_cflag & CBAUD);
}

speed_t cfgetospeed(const struct termios *termios_p) {
    if (!termios_p) {
        errno = EINVAL;
        return (speed_t)-1;
    }
    return (speed_t)(termios_p->c_cflag & CBAUD);
}

int cfsetispeed(struct termios *termios_p, speed_t speed) {
    if (!termios_p || speed > B4000000) {
        errno = EINVAL;
        return -1;
    }
    termios_p->c_cflag = (termios_p->c_cflag & ~CBAUD) | (speed & CBAUD);
    return 0;
}

int cfsetospeed(struct termios *termios_p, speed_t speed) {
    if (!termios_p || speed > B4000000) {
        errno = EINVAL;
        return -1;
    }
    termios_p->c_cflag = (termios_p->c_cflag & ~CBAUD) | (speed & CBAUD);
    return 0;
}

void cfmakeraw(struct termios *termios_p) {
    if (!termios_p) return;
    
    /* Input flags */
    termios_p->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    
    /* Output flags */
    termios_p->c_oflag &= ~OPOST;
    
    /* Local flags */
    termios_p->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    
    /* Control flags - 8-bit, no parity */
    termios_p->c_cflag &= ~(CSIZE | PARENB);
    termios_p->c_cflag |= CS8;
    
    /* No timeout, minimum 1 character */
    termios_p->c_cc[VMIN] = 1;
    termios_p->c_cc[VTIME] = 0;
}
