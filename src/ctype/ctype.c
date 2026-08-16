#include <ctype.h>

int isascii(int c) {
    return (unsigned)c < 128;
}

int toascii(int c) {
    return c & 0x7F;
}

int isdigit(int c) {
    return (unsigned)c - '0' < 10;
}

int isalpha(int c) {
    return (unsigned)((c | 32) - 'a') < 26;
}

int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

int isblank(int c) {
    return c == ' ' || c == '\t';
}

int iscntrl(int c) {
    return (unsigned)c < 32 || c == 127;
}

int isgraph(int c) {
    return (unsigned)c - '!' < 94;
}

int islower(int c) {
    return (unsigned)c - 'a' < 26;
}

int isupper(int c) {
    return (unsigned)c - 'A' < 26;
}

int isprint(int c) {
    return (unsigned)c - ' ' < 95;
}

int isspace(int c) {
    return c == ' ' || (unsigned)c - '\t' < 5;
}

int ispunct(int c) {
    return isprint(c) && !isalnum(c) && c != ' ';
}

int isxdigit(int c) {
    return isdigit(c) || (unsigned)((c | 32) - 'a') < 6;
}

int tolower(int c) {
    if (isupper(c)) return c | 32;
    return c;
}

int toupper(int c) {
    if (islower(c)) return c & ~32;
    return c;
}
