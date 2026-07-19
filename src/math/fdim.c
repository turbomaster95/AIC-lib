#include "libm.h"

double fdim(double x, double y) {
    if (isnan(x) || isnan(y))
        return NAN;
    return x > y ? x - y : 0.0;
}
