#include "libm.h"

double fmax(double x, double y) {
    if (isnan(x))
        return y;
    if (isnan(y))
        return x;
    return x > y ? x : y;
}
