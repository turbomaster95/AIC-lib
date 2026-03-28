#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>
#include <stdarg.h>

/* File descriptor type */
typedef struct {
    int fd;
    // You can add buffer fields here later
} FILE;

/* Standard file descriptors */
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

/* Standard file descriptor numbers */
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

/* EOF and error indicators */
#define EOF (-1)
#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2
#define BUFSIZ 1024

/* File position type */
typedef long fpos_t;

/* Function declarations */
int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int vprintf(const char *format, va_list ap);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
int putchar(int c);
int getchar(void);
int putc(int c, FILE *stream);
int getc(FILE *stream);
int fputc(int c, FILE *stream);
int fgetc(FILE *stream);
int fputs(const char *s, FILE *stream);
char *fgets(char *s, int size, FILE *stream);
void flush(void);
void fflush(FILE *stream);
int ferror(FILE *stream);
void clearerr(FILE *stream);
int feof(FILE *stream);

/* File operations */
FILE *fopen(const char *pathname, const char *mode);
FILE *fdopen(int fd, const char *mode);
FILE *freopen(const char *pathname, const char *mode, FILE *stream);
int fclose(FILE *stream);
int fileno(FILE *stream);

/* Formatted input */
int scanf(const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);
int sscanf(const char *s, const char *format, ...);

/* Buffering */
void setbuf(FILE *stream, char *buf);
int setvbuf(FILE *stream, char *buf, int mode, size_t size);

/* AIC Internal/Helper helpers */
void print_str(const char *s);
void print_hex(unsigned long n);

#endif
