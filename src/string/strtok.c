#include <string.h>

static char *last_token = 0;

char *strtok(char *str, const char *delim) {
    char *token_start;

    // If str is NULL, continue from the last saved position
    if (str == 0) {
        str = last_token;
    }

    if (str == 0 || *str == '\0') {
        return 0;
    }

    // Skip leading delimiters
    while (*str) {
        int is_delim = 0;
        for (const char *d = delim; *d; d++) {
            if (*str == *d) {
                is_delim = 1;
                break;
            }
        }
        if (!is_delim) break;
        str++;
    }

    if (*str == '\0') {
        last_token = 0;
        return 0;
    }

    token_start = str;

    // Find the end of the token
    while (*str) {
        int is_delim = 0;
        for (const char *d = delim; *d; d++) {
            if (*str == *d) {
                is_delim = 1;
                break;
            }
        }
        if (is_delim) {
            *str = '\0'; // Terminate the token
            last_token = str + 1;
            return token_start;
        }
        str++;
    }

    last_token = 0; // No more tokens left
    return token_start;
}
