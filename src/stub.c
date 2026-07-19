#include <stdint.h>

/* Stub for string errors */
const char *strerror(int errnum) {
    (void)errnum;
    return "Unknown error";
}

/* Stubs for long double floating point formatting helpers */
int __fpclassifyl(long double x) {
    (void)x;
    return 4; // FP_NORMAL (Usually 4 in standard headers, or 0 depending on your math.h)
}

int __signbitl(long double x) {
    (void)x;
    return 0; // Assume positive
}

long double modfl(long double x, long double *iptr) {
    *iptr = (long int)x;
    return x - *iptr;
}
