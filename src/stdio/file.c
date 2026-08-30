#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <internal/pal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define UNGETC_BUF_SIZE 4

typedef struct {
    int fd;
    int flags;
    int error;
    int eof;
    unsigned char ungetc_buf[UNGETC_BUF_SIZE];
    int ungetc_count;
} FILE_impl;

static FILE_impl file_table[32];
static int file_table_initialized = 0;

static void init_file_table(void) {
    if (!file_table_initialized) {
        for (int i = 0; i < 32; i++) {
            file_table[i].fd = -1;
            file_table[i].ungetc_count = 0;
        }
        file_table_initialized = 1;
    }
}

FILE *fopen(const char *pathname, const char *mode) {
    int flags = 0;
    int mode_bits = 0644;

    if (mode == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if (strcmp(mode, "r") == 0 || strcmp(mode, "rb") == 0) {
        flags = O_RDONLY;
    } else if (strcmp(mode, "w") == 0 || strcmp(mode, "wb") == 0) {
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    } else if (strcmp(mode, "a") == 0 || strcmp(mode, "ab") == 0) {
        flags = O_WRONLY | O_CREAT | O_APPEND;
    } else if (strcmp(mode, "r+") == 0 || strcmp(mode, "r+b") == 0) {
        flags = O_RDWR;
    } else if (strcmp(mode, "w+") == 0 || strcmp(mode, "w+b") == 0) {
        flags = O_RDWR | O_CREAT | O_TRUNC;
    } else if (strcmp(mode, "a+") == 0 || strcmp(mode, "a+b") == 0) {
        flags = O_RDWR | O_CREAT | O_APPEND;
    } else {
        errno = EINVAL;
        return NULL;
    }

    int fd = pal_open(pathname, flags, mode_bits);
    if (fd < 0) {
        return NULL;
    }

    init_file_table();

    for (int i = 0; i < 32; i++) {
        if (file_table[i].fd == -1) {
            file_table[i].fd = fd;
            file_table[i].flags = flags;
            file_table[i].error = 0;
            file_table[i].eof = 0;
            file_table[i].ungetc_count = 0;
            return (FILE *)&file_table[i];
        }
    }

    errno = EMFILE;
    pal_close(fd);
    return NULL;
}

FILE *fdopen(int fd, const char *mode) {
    (void)mode;
    init_file_table();

    for (int i = 0; i < 32; i++) {
        if (file_table[i].fd == -1) {
            file_table[i].fd = fd;
            file_table[i].flags = 0;
            file_table[i].error = 0;
            file_table[i].eof = 0;
            file_table[i].ungetc_count = 0;
            return (FILE *)&file_table[i];
        }
    }

    errno = EMFILE;
    return NULL;
}

int fclose(FILE *stream) {
    if (!stream) return EOF;
    FILE_impl *f = (FILE_impl *)stream;
    if (f->fd != -1) {
        pal_close(f->fd);
        f->fd = -1;
        f->ungetc_count = 0;
        return 0;
    }
    return EOF;
}

int fileno(FILE *stream) {
    if (stream == NULL) {
        errno = EBADF;
        return -1;
    }
    FILE_impl *f = (FILE_impl *)stream;
    return f->fd;
}

FILE *freopen(const char *pathname, const char *mode, FILE *stream) {
    if (stream != NULL) {
        fclose(stream);
    }
    return fopen(pathname, mode);
}

int fgetc(FILE *stream) {
    if (!stream) return EOF;
    FILE_impl *f = (FILE_impl *)stream;

    if (f->ungetc_count > 0) {
        return f->ungetc_buf[--f->ungetc_count];
    }

    char c;
    long ret = pal_read(f->fd, &c, 1);
    if (ret <= 0) {
        f->eof = 1;
        return EOF;
    }
    return (unsigned char)c;
}

int getc(FILE *stream) {
    return fgetc(stream);
}

int getc_unlocked(FILE *stream) {
    return fgetc(stream);
}

int ungetc(int c, FILE *stream) {
    if (c == EOF || !stream) return EOF;
    FILE_impl *f = (FILE_impl *)stream;

    if (f->ungetc_count >= UNGETC_BUF_SIZE) {
        return EOF;
    }

    f->ungetc_buf[f->ungetc_count++] = (unsigned char)c;
    f->eof = 0;
    return (unsigned char)c;
}

char *fgets(char *s, int size, FILE *stream) {
    if (!s || size <= 0 || !stream) return NULL;
    int i = 0;
    while (i < size - 1) {
        int ch = fgetc(stream);
        if (ch == EOF) {
            if (i == 0) return NULL;
            break;
        }
        s[i++] = (char)ch;
        if (ch == '\n') break;
    }
    s[i] = '\0';
    return s;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    if (!ptr || size == 0 || nmemb == 0 || !stream) return 0;
    size_t total_bytes = size * nmemb;
    size_t bytes_read = 0;
    char *buf = (char *)ptr;

    while (bytes_read < total_bytes) {
        int ch = fgetc(stream);
        if (ch == EOF) break;
        buf[bytes_read++] = (char)ch;
    }

    return bytes_read / size;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    if (!ptr || !stream || size == 0 || nmemb == 0) return 0;
    FILE_impl *f = (FILE_impl *)stream;
    size_t total = size * nmemb;
    long ret = pal_write(f->fd, ptr, total);
    if (ret < 0) {
        f->error = 1;
        return 0;
    }
    return (size_t)ret / size;
}

int fseek(FILE *stream, long offset, int whence) {
    if (!stream) return -1;
    FILE_impl *f = (FILE_impl *)stream;
    f->ungetc_count = 0;
    long ret = pal_lseek(f->fd, offset, whence);
    if (ret < 0) {
        f->error = 1;
        return -1;
    }
    f->eof = 0;
    return 0;
}

long ftell(FILE *stream) {
    if (!stream) return -1;
    FILE_impl *f = (FILE_impl *)stream;
    long pos = pal_lseek(f->fd, 0, SEEK_CUR);
    if (pos < 0) {
        f->error = 1;
        return -1;
    }
    return pos - f->ungetc_count;
}

int fseeko(FILE *stream, off_t offset, int whence) {
    return fseek(stream, (long)offset, whence);
}

off_t ftello(FILE *stream) {
    return (off_t)ftell(stream);
}

int setvbuf(FILE *stream, char *buf, int mode, size_t size) {
    (void)stream; (void)buf; (void)mode; (void)size;
    return 0;
}

void flockfile(FILE *stream) { (void)stream; }
void funlockfile(FILE *stream) { (void)stream; }
int ftrylockfile(FILE *stream) { (void)stream; return 0; }

FILE *popen(const char *command, const char *type) {
    (void)command; (void)type;
    errno = ENOSYS;
    return NULL;
}

int pclose(FILE *stream) {
    (void)stream;
    return -1;
}

FILE *tmpfile(void) {
    errno = ENOSYS;
    return NULL;
}
