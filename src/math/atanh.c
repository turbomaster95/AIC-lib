#include "libm.h"

double atanh(double x) {
    if (isnan(x)) return x;

    if (x == 1.0)  return INFINITY;
    if (x == -1.0) return -INFINITY;

    if (x > 1.0 || x < -1.0) {
        return NAN;
    }

    if (x == 0.0) return x;

    double ax = fabs(x);
    if (ax < 1e-4) {
        double x2 = x * x;
        return x + (x * x2) / 3.0 + (x * x2 * x2) / 5.0;
    }

    double t1 = 1.0 + x;
    double t2 = 1.0 - x;
    return 0.5 * log(t1 / t2);
}
