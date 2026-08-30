#include <stdlib.h>
#include <ctype.h>

double strtod(const char *nptr, char **endptr) {
    const char *s = nptr;
    double val = 0.0;
    int neg = 0;

    while (isspace((unsigned char)*s)) s++;

    if (*s == '-') {
        neg = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    while (isdigit((unsigned char)*s)) {
        val = val * 10.0 + (*s - '0');
        s++;
    }

    if (*s == '.') {
        s++;
        double frac = 1.0;
        while (isdigit((unsigned char)*s)) {
            frac /= 10.0;
            val += (*s - '0') * frac;
            s++;
        }
    }

    if (*s == 'e' || *s == 'E') {
        s++;
        int exp_neg = 0;
        int exp_val = 0;

        if (*s == '-') {
            exp_neg = 1;
            s++;
        } else if (*s == '+') {
            s++;
        }

        while (isdigit((unsigned char)*s)) {
            exp_val = exp_val * 10 + (*s - '0');
            s++;
        }

        double scale = 1.0;
        for (int i = 0; i < exp_val; i++) {
            scale *= 10.0;
        }

        if (exp_neg) {
            val /= scale;
        } else {
            val *= scale;
        }
    }

    if (endptr) {
        *endptr = (char *)s;
    }

    return neg ? -val : val;
}
