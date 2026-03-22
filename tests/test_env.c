#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
    printf("--- Environment Variables ---\n");
    
    if (environ == NULL) {
        printf("Error: environ is NULL\n");
        return 1;
    }

    for (int i = 0; environ[i] != NULL; i++) {
        printf("%s\n", environ[i]);
    }

    return 0;
}
