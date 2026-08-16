#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <internal/pal.h>

struct __DIR {
    int fd;
    size_t buf_pos;
    size_t buf_end;
    char buf[2048];
};

struct linux_dirent64 {
    ino_t          d_ino;
    off_t          d_off;
    unsigned short d_reclen;
    unsigned char  d_type;
    char           d_name[];
};

DIR *opendir(const char *name) {
    int fd = open(name, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (fd < 0) return NULL;

    DIR *dir = malloc(sizeof(DIR));
    if (!dir) {
        close(fd);
        return NULL;
    }
    dir->fd = fd;
    dir->buf_pos = 0;
    dir->buf_end = 0;
    return dir;
}

DIR *fdopendir(int fd) {
    DIR *dir = malloc(sizeof(DIR));
    if (!dir) return NULL;
    dir->fd = fd;
    dir->buf_pos = 0;
    dir->buf_end = 0;
    return dir;
}

struct dirent *readdir(DIR *dirp) {
    static struct dirent de;

    if (dirp->buf_pos >= dirp->buf_end) {
        long nread = pal_getdents64(dirp->fd, dirp->buf, sizeof(dirp->buf));
        if (nread <= 0) return NULL;
        dirp->buf_end = (size_t)nread;
        dirp->buf_pos = 0;
    }

    struct linux_dirent64 *ld = (struct linux_dirent64 *)(dirp->buf + dirp->buf_pos);
    dirp->buf_pos += ld->d_reclen;

    de.d_ino = ld->d_ino;
    de.d_off = ld->d_off;
    de.d_reclen = ld->d_reclen;
    de.d_type = ld->d_type;

    size_t i = 0;
    while (ld->d_name[i] && i < sizeof(de.d_name) - 1) {
        de.d_name[i] = ld->d_name[i];
        i++;
    }
    de.d_name[i] = '\0';

    return &de;
}

int closedir(DIR *dirp) {
    if (!dirp) return -1;
    int ret = close(dirp->fd);
    free(dirp);
    return ret;
}

int dirfd(DIR *dirp) {
    return dirp->fd;
}
