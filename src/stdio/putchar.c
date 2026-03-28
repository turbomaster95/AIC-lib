#include <internal/syscall.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>

#define BUF_SIZE 1024
static char stdout_buf[BUF_SIZE];
static int buf_idx = 0;

void flush(void) {
    if (buf_idx > 0) {
        long ret = __syscall3(SYS_write, STDOUT_FILENO, (long)stdout_buf, (long)buf_idx);
        if (ret < 0) {
            /* Error handling could set errno here */
        }
        buf_idx = 0;
    }
}

int putchar(int c) {
    return putc(c, stdout);
}

int putc(int c, FILE *stream) {
    return fputc(c, stream);
}

int fputc(int c, FILE *stream) {
    int fd = (stream != NULL) ? stream->fd : STDOUT_FILENO;    
    /* stderr is unbuffered */
    if (fd == STDERR_FILENO) {
        char ch = (char)c;
        long ret = __syscall3(SYS_write, fd, (long)&ch, 1);
        if (ret < 0) {
            return EOF;
        }
        return c;
    }
    
    /* stdout and other streams use buffering */
    if (buf_idx >= BUF_SIZE) {
        flush();
    }
    stdout_buf[buf_idx++] = (char)c;
    
    /* Flush on newline for line-buffered behavior */
    if (c == '\n') {
        flush();
    }
    
    return c;
}

int fputs(const char *s, FILE *stream) {
    int fd = (stream != NULL) ? stream->fd : STDOUT_FILENO;
    size_t len = 0;
    const char *p = s;
    
    while (*p) p++;
    len = (size_t)(p - s);
    
    if (len == 0) return 0;
    
    long ret = __syscall3(SYS_write, fd, (long)s, (long)len);
    if (ret < 0) {
        return EOF;
    }
    
    return 0;
}

int fprintf(FILE *stream, const char *format, ...) {
    char buffer[1024];
    va_list ap;
    va_start(ap, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, ap);
    va_end(ap);
    
    if (stream == stdout) {
        print_str(buffer);
    } else {
        fputs(buffer, stream);
    }
    
    return len;
}

int vprintf(const char *format, va_list ap) {
    char buffer[1024];
    int len = vsnprintf(buffer, sizeof(buffer), format, ap);
    print_str(buffer);
    return len;
}
