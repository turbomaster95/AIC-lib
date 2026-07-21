#include <internal/pal.h>
#include <stdlib.h>
#include <stdio.h>

void exit(int code) {
    flush(); // Ensure all buffered output hits the screen
    pal_exit(code);
    for(;;);
}
