#ifndef _TERMIOS_H
#define _TERMIOS_H

#include <bits/types.h>

/* Speed type */
typedef unsigned int speed_t;

/* Baud rates */
#define B0       0
#define B50      1
#define B75      2
#define B110     3
#define B134     4
#define B150     5
#define B200     6
#define B300     7
#define B600     8
#define B1200    9
#define B1800    10
#define B2400    11
#define B4800    12
#define B9600    13
#define B19200   14
#define B38400   15
#define B57600   16
#define B115200  17
#define B230400  18
#define B460800  19
#define B500000  20
#define B576000  21
#define B921600  22
#define B1000000 23
#define B1152000 24
#define B1500000 25
#define B2000000 26
#define B2500000 27
#define B3000000 28
#define B3500000 29
#define B4000000 30

/* Control characters */
#define VINTR    0
#define VQUIT    1
#define VERASE   2
#define VKILL    3
#define VEOF     4
#define VTIME    5
#define VMIN     6
#define VSWTC    7
#define VSTART   8
#define VSTOP    9
#define VSUSP    10
#define VEOL     11
#define VREPRINT 12
#define VDISCARD 13
#define VWERASE  14
#define VLNEXT   15
#define VEOL2    16
#define NCCS     19

/* Input flags */
#define IGNBRK  0000001
#define BRKINT  0000002
#define IGNPAR  0000004
#define PARMRK  0000010
#define INPCK   0000020
#define ISTRIP  0000040
#define INLCR   0000100
#define IGNCR   0000200
#define ICRNL   0000400
#define IUCLC   0001000
#define IXON    0002000
#define IXANY   0004000
#define IXOFF   0010000
#define IMAXBEL 0020000
#define IUTF8   0040000

/* Output flags */
#define OPOST   0000001
#define OLCUC   0000002
#define ONLCR   0000004
#define OCRNL   0000010
#define ONOCR   0000020
#define ONLRET  0000040
#define OFILL   0000100
#define OFDEL   0000200
#define NLDLY   0000400
#define NL0     0000000
#define NL1     0000400
#define CRDLY   0003000
#define CR0     0000000
#define CR1     0001000
#define CR2     0002000
#define CR3     0003000
#define TABDLY  0014000
#define TAB0    0000000
#define TAB1    0004000
#define TAB2    0010000
#define TAB3    0014000
#define BSDLY   0020000
#define BS0     0000000
#define BS1     0020000
#define FFDLY   0100000
#define FF0     0000000
#define FF1     0100000
#define VTDLY   0040000
#define VT0     0000000
#define VT1     0040000

/* Control flags */
#define CBAUD   0010017
#define CBAUDEX 0010000
#define CSIZE   0000060
#define CS5     0000000
#define CS6     0000020
#define CS7     0000040
#define CS8     0000060
#define CSTOPB  0000100
#define CREAD   0000200
#define PARENB  0000400
#define PARODD  0001000
#define HUPCL   0002000
#define CLOCAL  0004000
#define CIBAUD  03600000
#define CRTSCTS 020000000000

/* Local flags */
#define ISIG    0000001
#define ICANON  0000002
#define XCASE   0000004
#define ECHO    0000010
#define ECHOE   0000020
#define ECHOK   0000040
#define ECHONL  0000100
#define NOFLSH  0000200
#define TOSTOP  0000400
#define ECHOCTL 0001000
#define ECHOPRT 0002000
#define ECHOKE  0004000
#define FLUSHO  0010000
#define PENDIN  0040000
#define IEXTEN  0100000

/* TC actions */
#define TCOOFF  0
#define TCOON   1
#define TCIOFF  2
#define TCION   3
#define TCFLSH  0x5407
#define TCSBRK  0x5409
#define TCXONC  0x540A

/* TC drain actions */
#define TCIFLUSH  0
#define TCOFLUSH  1
#define TCIOFLUSH 2

/* TC set actions */
#define TCSANOW   0
#define TCSADRAIN 1
#define TCSAFLUSH 2

/* ioctl commands for terminal */
#define TCGETS      0x5401
#define TCSETS      0x5402
#define TCSETSW     0x5403
#define TCSETSF     0x5404
#define TIOCGWINSZ  0x5413
#define TIOCSWINSZ  0x5414
#define TIOCGPGRP   0x540F
#define TIOCSPGRP   0x5410
#define TIOCGPTN    0x80045430
#define TIOCSCTTY   0x540E

/* winsize structure */
struct winsize {
    unsigned short ws_row;
    unsigned short ws_col;
    unsigned short ws_xpixel;
    unsigned short ws_ypixel;
};

/* termios structure */
struct termios {
    unsigned long c_iflag;
    unsigned long c_oflag;
    unsigned long c_cflag;
    unsigned long c_lflag;
    unsigned char c_line;
    unsigned char c_cc[NCCS];
    unsigned long c_ispeed;
    unsigned long c_ospeed;
};

/* Function declarations */
int ioctl(int fd, unsigned long request, ...);
int tcgetattr(int fd, struct termios *termios_p);
int tcsetattr(int fd, int optional_actions, const struct termios *termios_p);
int tcflush(int fd, int queue_selector);
int tcflow(int fd, int action);
int tcsendbreak(int fd, int duration);
int tcdrain(int fd);
speed_t cfgetispeed(const struct termios *termios_p);
speed_t cfgetospeed(const struct termios *termios_p);
int cfsetispeed(struct termios *termios_p, speed_t speed);
int cfsetospeed(struct termios *termios_p, speed_t speed);
void cfmakeraw(struct termios *termios_p);

#endif /* _TERMIOS_H */
