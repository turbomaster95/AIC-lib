#include "libm.h"

long double modfl(long double x, long double *iptr) {
    int cls = __fpclassifyl(x);

    if (cls == FP_NAN) {
        *iptr = x;
        return x;
    }
    if (cls == FP_INFINITE) {
        *iptr = x;
        return __signbitl(x) ? -0.0L : 0.0L;
    }

    if (x >= 9223372036854775808.0L || x <= -9223372036854775808.0L) {
        *iptr = x;
        return __signbitl(x) ? -0.0L : 0.0L;
    }

    long long i = (long long)x;
    long double integer_part = (long double)i;

    if (i == 0 && __signbitl(x)) {
        *iptr = -0.0L;
    } else {
        *iptr = integer_part;
    }

    return x - *iptr;
}
