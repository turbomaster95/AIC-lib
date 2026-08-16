#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H

#include <bits/alltypes.h>

/* Term size struct used by TIOCGWINSZ / TIOCSWINSZ */
struct winsize {
    unsigned short ws_row;
    unsigned short ws_col;
    unsigned short ws_xpixel;
    unsigned short ws_ypixel;
};

/* Term I/O commands */
#define TCGETS          0x5401
#define TCSETS          0x5402
#define TCSETSW         0x5403
#define TCSETSF         0x5404
#define TIOCGWINSZ      0x5413
#define TIOCSWINSZ      0x5414
#define TIOCGPGRP       0x540F
#define TIOCSPGRP       0x5410
#define TIOCSCTTY       0x540E
#define TIOCNOTTY       0x5422
#define FIONREAD        0x541B
#define FIONBIO         0x5421
#define FIONCLEX        0x5451
#define FIOCLEX         0x5450

int ioctl(int fd, unsigned long request, ...);

#endif /* _SYS_IOCTL_H */
