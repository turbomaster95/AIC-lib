#include <stdio.h>

int main(int argc, char **argv) {
    printf("AIC OS Loader Active\n");
    printf("Arg count: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
    return 0;
}
