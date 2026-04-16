#include <stdio.h>
#include <unistd.h>
#include <string.h>

int puts(const char *s) {
    if (!s) return -1;
    write(STDOUT_FILENO, s, strlen(s));
    write(STDOUT_FILENO, "\n", 1);
    return 0;
}
