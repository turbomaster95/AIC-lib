#include <unistd.h>
#include <internal/syscall.h>
#include <string.h>

#ifdef __x86_64_efi__
void clearScreen(void) {
    __syscall0(SYS_clearScreen);
}
#else
void clearScreen(void) {
    const char *seq = "\033[H\033[2J"; /* ESC [ H  ESC [ 2 J */
    size_t len = strlen(seq);
    ssize_t written = write(STDOUT_FILENO, seq, len);
    (void)written; /* ignore write errors for this minimal example */
}
#endif
