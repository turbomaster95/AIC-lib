#ifndef _FCNTL_H
#define _FCNTL_H

#include <bits/types.h>

/* File offset type */
typedef long off_t;

/* Special fd values for *at functions */
#define AT_FDCWD (-100)

/* File access modes */
#define O_RDONLY    0x0000      /* Open for reading only */
#define O_WRONLY    0x0001      /* Open for writing only */
#define O_RDWR      0x0002      /* Open for reading and writing */
#define O_ACCMODE   0x0003      /* Mask for file access mode */

/* File creation flags */
#define O_CREAT     0x0040      /* Create file if it doesn't exist */
#define O_EXCL      0x0080      /* Exclusive use - fail if file exists */
#define O_NOCTTY    0x0100      /* Don't make this terminal */
#define O_TRUNC     0x0200      /* Truncate file to zero length */
#define O_APPEND    0x0400      /* Append to file */
#define O_NONBLOCK  0x0800      /* Non-blocking mode */
#define O_DSYNC     0x1000      /* Synchronize data */
#define O_SYNC      0x2000      /* Synchronize I/O */
#define O_RSYNC     0x4000      /* Synchronize reads */
#define O_BINARY    0x8000      /* Binary mode (Windows compat) */
#define O_TEXT      0x0000      /* Text mode (Windows compat) */

/* File status flags for fcntl */
#define F_DUPFD     0           /* Duplicate file descriptor */
#define F_GETFD     1           /* Get file descriptor flags */
#define F_SETFD     2           /* Set file descriptor flags */
#define F_GETFL     3           /* Get file status flags */
#define F_SETFL     4           /* Set file status flags */
#define F_GETLK     5           /* Get record locking info */
#define F_SETLK     6           /* Set record locking info */
#define F_SETLKW    7           /* Set record locking info (wait) */

/* File descriptor flags */
#define FD_CLOEXEC  1           /* Close on exec */

/* File locking types */
#define F_RDLCK     0           /* Read lock */
#define F_WRLCK     1           /* Write lock */
#define F_UNLCK     2           /* Unlock */

/* SEEK macros (also in unistd.h) */
#ifndef _UNISTD_H
#define SEEK_SET    0           /* Seek from beginning */
#define SEEK_CUR    1           /* Seek from current position */
#define SEEK_END    2           /* Seek from end */
#endif

/* Mode constants for open() */
#define S_IFMT      0170000     /* File type mask */
#define S_IFREG     0100000     /* Regular file */
#define S_IFDIR     0040000     /* Directory */
#define S_IFCHR     0020000     /* Character device */
#define S_IFBLK     0060000     /* Block device */
#define S_IFIFO     0010000     /* FIFO */
#define S_IFLNK     0120000     /* Symbolic link */
#define S_IFSOCK    0140000     /* Socket */

#define S_IRWXU     00700       /* Owner read/write/execute */
#define S_IRUSR     00400       /* Owner read */
#define S_IWUSR     00200       /* Owner write */
#define S_IXUSR     00100       /* Owner execute */
#define S_IRWXG     00070       /* Group read/write/execute */
#define S_IRGRP     00040       /* Group read */
#define S_IWGRP     00020       /* Group write */
#define S_IXGRP     00010       /* Group execute */
#define S_IRWXO     00007       /* Others read/write/execute */
#define S_IROTH     00004       /* Others read */
#define S_IWOTH     00002       /* Others write */
#define S_IXOTH     00001       /* Others execute */

/* flock structure for file locking */
struct flock {
    short l_type;               /* Lock type (F_RDLCK, F_WRLCK, F_UNLCK) */
    short l_whence;             /* Type for l_start (SEEK_SET, SEEK_CUR, SEEK_END) */
    off_t l_start;              /* Starting offset for lock */
    off_t l_len;                /* Length of lock (0 = EOF) */
    pid_t l_pid;                /* PID of process holding lock */
};

/* Function declarations */
int open(const char *pathname, int flags, ...);
int creat(const char *pathname, int mode);
int fcntl(int fd, int cmd, ...);
int flock(int fd, int operation);

#endif /* _FCNTL_H */
