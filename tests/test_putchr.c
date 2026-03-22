#define USING_STD_H_OK
#include <std.h>

int main(int argc, char **argv) {
    printf("Testing Buffered Output...\n");
    
    for(int i = 0; i < 1000; i++) {
        putchar('.');
        if (i % 50 == 49) putchar('\n');
    }
    
    exit(0); 
}
