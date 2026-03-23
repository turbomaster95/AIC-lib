#include <string.h>
#include <limits.h>

char *itoa(long val, char *buf, int base) {
    static const char digits[] = "0123456789abcdef";
    char *p = buf;
    char *p1, *p2;
    unsigned long uval;
    int negative = 0;

    /* Handle base 10 negative numbers */
    if (base == 10 && val < 0) {
        negative = 1;
        /* Cast to unsigned first to avoid overflow on LONG_MIN */
        uval = (unsigned long)(-(val + 1)) + 1;
    } else {
        uval = (unsigned long)val;
    }

    /* Add sign if needed */
    if (negative) {
        *p++ = '-';
    }

    p1 = p;
    
    /* Handle zero case */
    if (uval == 0) {
        *p++ = '0';
        *p = '\0';
        return buf;
    }
    
    /* Convert digits in reverse order */
    while (uval > 0) {
        *p++ = digits[uval % (unsigned long)base];
        uval /= (unsigned long)base;
    }

    *p = '\0';
    p2 = p - 1;

    /* Reverse the digits in place */
    while (p1 < p2) {
        char tmp = *p1;
        *p1++ = *p2;
        *p2-- = tmp;
    }
    
    return buf;
}
