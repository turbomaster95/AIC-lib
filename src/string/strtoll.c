#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

long long strtoll(const char *nptr, char **endptr, int base) {
    const char *s = nptr;
    unsigned long long acc = 0;
    int c;
    unsigned long long cutoff;
    int neg = 0, any, cutlim;

    do {
        c = *s++;
    } while (isspace((unsigned char)c));

    if (c == '-') {
        neg = 1;
        c = *s++;
    } else if (c == '+') {
        c = *s++;
    }

    if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0) {
        base = (c == '0') ? 8 : 10;
    }

    cutoff = neg ? -(unsigned long long)LLONG_MIN : LLONG_MAX;
    cutlim = cutoff % (unsigned long long)base;
    cutoff /= (unsigned long long)base;

    for (acc = 0, any = 0;; c = *s++) {
        if (isdigit((unsigned char)c)) {
            c -= '0';
        } else if (isalpha((unsigned char)c)) {
            c -= isupper((unsigned char)c) ? 'A' - 10 : 'a' - 10;
        } else {
            break;
        }

        if (c >= base) {
            break;
        }

        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
            any = -1;
        } else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }

    if (any < 0) {
        acc = neg ? LLONG_MIN : LLONG_MAX;
        errno = ERANGE;
    } else if (neg) {
        acc = -acc;
    }

    if (endptr != NULL) {
        *endptr = (char *)(any ? s - 1 : nptr);
    }

    return acc;
}
