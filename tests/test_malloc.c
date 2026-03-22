#define USING_STD_H_OK
#include <std.h>

int main(int argc, char **argv) {
    printf("AIC Memory Stress Test\n");

    for (int i = 1; i <= 5; i++) {
        char *p = malloc(i * 100);
        if (p) {
            printf("Allocated %d bytes at %p\n", i * 100, p);
            // Verify we can write to it
            p[0] = 'A';
            free(p);
        }
    }

    return 0;
}
