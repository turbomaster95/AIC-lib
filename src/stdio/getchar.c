#include <internal/syscall.h>
#include <stdio.h>

#define IN_BUF_SIZE 1024
static char in_buf[IN_BUF_SIZE];
static int in_ptr = 0;
static int in_len = 0;

int getchar(void) {
    // If buffer is empty, refill it from stdin
    if (in_ptr >= in_len) {
        long n = __syscall3(SYS_read, 0, (long)in_buf, IN_BUF_SIZE);
        if (n <= 0) return -1; // EOF or Error
        in_len = (int)n;
        in_ptr = 0;
    }
    
    return (unsigned char)in_buf[in_ptr++];
}
