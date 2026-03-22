#include <string.h>

char *itoa(long val, char *buf, int base) {
    static char digits[] = "0123456789abcdef";
    char *p = buf;
    char *p1, *p2;
    unsigned long uval = val;

    if (base == 10 && val < 0) {
        *p++ = '-';
        uval = -val;
    }

    p1 = p;
    do {
        *p++ = digits[uval % base];
        uval /= base;
    } while (uval > 0);

    *p = '\0';
    p2 = p - 1;

    // Reverse the string in place
    while (p1 < p2) {
        char tmp = *p1;
        *p1++ = *p2;
        *p2-- = tmp;
    }
    return buf;
}
