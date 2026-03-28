#include <unistd.h>
#include <internal/syscall.h>

void clearScreen(void) {
    __syscall0(SYS_clearScreen);
}
