#include "libm.h"

double tanh(double x) {
    double_t r, s, z, t;
    int32_t ix, sign;

    GET_HIGH_WORD(ix, x);
    sign = ix >> 31;
    ix &= 0x7fffffff;

    if (ix >= 0x7ff00000)
        return x - x;
    if (ix >= 0x40360000) {
        r = 1.0 - 0.0;
        return sign ? -r : r;
    }
    if (ix >= 0x40800000)
        return 1.0 / expm1(2 * fabs(x)) + sign;
    if (ix >= 0x3fc00000) {
        t = expm1(2 * fabs(x));
        r = 1.0 - 2.0 / (t + 2.0);
        return sign ? -r : r;
    }
    if (ix < 0x3e300000) {
        if (ix == 0)
            return x;
        r = x * x;
        return x + x * r * (0.33333333333333333 + r * (0.13333333333333333 + r * 0.05396825396825397));
    }
    r = expm1(-2 * fabs(x));
    z = 1.0 - 2.0 / (r + 2.0);
    return sign ? -z : z;
}
