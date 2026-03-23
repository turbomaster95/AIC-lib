#include <stdio.h>

/* 
 * Standard file descriptors
 * In AIC, FILE* is just an int wrapper for simplicity.
 * These point to the standard POSIX file descriptors.
 */
static FILE _stdin  = STDIN_FILENO;
static FILE _stdout = STDOUT_FILENO;
static FILE _stderr = STDERR_FILENO;

FILE *stdin  = &_stdin;
FILE *stdout = &_stdout;
FILE *stderr = &_stderr;

/* Error state for each stream (simplified - just one global for now) */
static int _ferror = 0;
static int _feof = 0;

int ferror(FILE *stream) {
    (void)stream;
    return _ferror;
}

void clearerr(FILE *stream) {
    (void)stream;
    _ferror = 0;
    _feof = 0;
}

int feof(FILE *stream) {
    (void)stream;
    return _feof;
}

void fflush(FILE *stream) {
    if (stream == stdout || stream == NULL) {
        flush();
    }
    /* stderr is unbuffered, nothing to flush */
}
