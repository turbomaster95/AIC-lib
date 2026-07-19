#include "libm.h"

static const double
    ivln2hi = 1.44269504088896338700e+00,
    ivln2lo = 1.67517131648865107581e-10;

double log2(double x) {
    if (x <= 0)
        return -1 / (x - x);
    return __log(x) * ivln2hi + __log(x) * ivln2lo;
}
