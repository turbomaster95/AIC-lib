#include "libm.h"

double cosh(double x) {
    double_t t;
    uint32_t ix;

    GET_HIGH_WORD(ix, x);
    ix &= 0x7fffffff;
    if (ix >= 0x7ff00000)
        return x * x;
    if (ix < 0x3e300000)
        return 1.0;
    if (ix > 0x405db000) {
        if (ix >= 0x40862e42)
            return exp(0.5 * x) * 0x5e17;
        t = exp(0.5 * x);
        return t * t;
    }
    t = exp(x);
    return 0.5 * (t + 1.0 / t);
}
