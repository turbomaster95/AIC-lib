#include <internal/syscall.h>
#include <stdlib.h>
#include <stdio.h>

void exit(int code) {
    flush(); // Ensure all buffered output hits the screen
    __syscall3(SYS_exit, code, 0, 0);
    for(;;);
}
