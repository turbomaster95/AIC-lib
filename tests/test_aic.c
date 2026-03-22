#define USING_STD_H_OK
#include <std.h>

int main(int argc, char **argv) {
    void *p = malloc(128);
    printf("AIC Libc Loaded!\n");
    printf("Arg count: %d\n", argc);
    printf("Malloc'd address: 0x%p\n", p);
    for(char c = 'A'; c <= 'Z'; c++) {
        putchar(c);
    }

    putchar('\n');
    return 0;
}
