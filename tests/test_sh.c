#define USING_STD_H_OK
#include <std.h>

int main() {
    char input[1024];
    char *args[64];

    printf("\033[1;32mAIC Shell\033[0m (Arch/Termux)\n");

    while (1) {
        printf("aic$ ");
        
        // Read line
        int i = 0, c;
        while ((c = getchar()) != '\n' && c != -1 && i < 1023) {
            input[i++] = (char)c;
        }
        input[i] = '\0';

        if (i == 0) continue;

        // Parse arguments
        int count = 0;
        char *tok = strtok(input, " ");
        while (tok && count < 63) {
            args[count++] = tok;
            tok = strtok(NULL, " ");
        }
        args[count] = NULL;

        if (strcmp(args[0], "exit") == 0) break;

        // Execution logic
        long pid = fork();
        if (pid == 0) {
            execve(args[0], args, NULL);
            printf("aic: command not found: %s\n", args[0]);
            exit(1);
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
    }
    return 0;
}
