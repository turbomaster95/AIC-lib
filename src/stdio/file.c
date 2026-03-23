#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <internal/syscall.h>
#include <errno.h>
#include <string.h>

/* Simple FILE wrapper - in a real libc this would be more complex */
typedef struct {
    int fd;
    int flags;
    int error;
    int eof;
} FILE_impl;

/* We need to store FILE_impl somewhere - for now use a simple approach */
/* In production code, you'd want a proper file descriptor table */

static FILE_impl file_table[32];
static int file_table_initialized = 0;

FILE *fopen(const char *pathname, const char *mode) {
    int flags = 0;
    int mode_bits = 0644;  /* Default permissions: rw-r--r-- */
    
    /* Parse mode string */
    if (mode == NULL) {
        errno = EINVAL;
        return NULL;
    }
    
    /* Basic mode parsing */
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
    
    int fd = open(pathname, flags, mode_bits);
    if (fd < 0) {
        return NULL;
    }
    
    /* Find a free slot in file table */
    if (!file_table_initialized) {
        for (int i = 0; i < 32; i++) {
            file_table[i].fd = -1;
        }
        file_table_initialized = 1;
    }
    
    for (int i = 0; i < 32; i++) {
        if (file_table[i].fd == -1) {
            file_table[i].fd = fd;
            file_table[i].flags = flags;
            file_table[i].error = 0;
            file_table[i].eof = 0;
            return (FILE *)&file_table[i];
        }
    }
    
    /* No free slots */
    errno = EMFILE;
    close(fd);
    return NULL;
}

FILE *fdopen(int fd, const char *mode) {
    /* Similar to fopen but uses existing fd */
    (void)mode;  /* Mode parsing omitted for brevity */
    
    if (!file_table_initialized) {
        for (int i = 0; i < 32; i++) {
            file_table[i].fd = -1;
        }
        file_table_initialized = 1;
    }
    
    for (int i = 0; i < 32; i++) {
        if (file_table[i].fd == -1) {
            file_table[i].fd = fd;
            file_table[i].flags = 0;
            file_table[i].error = 0;
            file_table[i].eof = 0;
            return (FILE *)&file_table[i];
        }
    }
    
    errno = EMFILE;
    return NULL;
}

int fclose(FILE *stream) {
    if (stream == NULL) {
        return EOF;
    }
    
    FILE_impl *f = (FILE_impl *)stream;
    
    /* Flush if writable */
    if (f->fd == STDOUT_FILENO || f->fd == STDERR_FILENO) {
        fflush(stream);
    }
    
    /* Close the fd */
    if (f->fd >= 0 && f->fd < 32) {
        __syscall1(SYS_close, f->fd);
        file_table[f->fd].fd = -1;
    }
    
    return 0;
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
    /* Close existing stream */
    if (stream != NULL) {
        fclose(stream);
    }
    
    /* Open new stream */
    return fopen(pathname, mode);
}

int fgetc(FILE *stream) {
    char c;
    int fd = (stream != NULL) ? ((FILE_impl *)stream)->fd : STDIN_FILENO;
    
    long ret = __syscall3(SYS_read, fd, (long)&c, 1);
    if (ret <= 0) {
        ((FILE_impl *)stream)->eof = 1;
        return EOF;
    }
    
    return (unsigned char)c;
}

int getc(FILE *stream) {
    return fgetc(stream);
}

char *fgets(char *s, int size, FILE *stream) {
    int fd = (stream != NULL) ? ((FILE_impl *)stream)->fd : STDIN_FILENO;
    int i = 0;
    
    while (i < size - 1) {
        char c;
        long ret = __syscall3(SYS_read, fd, (long)&c, 1);
        if (ret <= 0) {
            ((FILE_impl *)stream)->eof = 1;
            if (i == 0) return NULL;
            break;
        }
        s[i++] = c;
        if (c == '\n') break;
    }
    
    s[i] = '\0';
    return s;
}
