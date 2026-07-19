#include "libm.h"

static const double
    ivln10hi = 4.34294481878168880939e-01,
    ivln10lo = 2.24888728673641697297e-15;

double log10(double x) {
    if (x <= 0)
        return -1 / (x - x);
    return __log(x) * ivln10hi + __log(x) * ivln10lo;
}
