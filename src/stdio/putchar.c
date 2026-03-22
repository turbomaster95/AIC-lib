#include <internal/syscall.h>
#include <stdio.h>

#define BUF_SIZE 1024
static char stdout_buf[BUF_SIZE];
static int buf_idx = 0;

void flush() {
    if (buf_idx > 0) {
        __syscall3(SYS_write, 1, (long)stdout_buf, buf_idx);
        buf_idx = 0;
    }
}

int putchar(int c) {
    stdout_buf[buf_idx++] = (char)c;

    // Flush if we see a newline or buffer is full
    if (c == '\n' || buf_idx >= BUF_SIZE) {
        flush();
    }
    
    return c;
}
