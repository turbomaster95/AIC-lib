#define USING_STD_H_OK
#include <std.h>

int main(void) {
    printf("AIC Interactive Shell (Type something and hit Enter):\n");
    printf("> ");

    int c;
    while ((c = getchar()) != -1) {
        if (c == '\n') {
            printf("\n> ");
        } else {
            // Echo back uppercase just to prove we processed it
            if (c >= 'a' && c <= 'z') c -= 32;
            putchar(c);
        }
    }

    printf("\nGoodbye!\n");
    return 0;
}
