#include "libm.h"

double asinh(double x) {
    double_t t, w;
    int32_t hx, ix;

    GET_HIGH_WORD(hx, x);
    ix = hx & 0x7fffffff;
    if (ix >= 0x7ff00000)
        return x + x;
    if (ix < 0x3e300000) {
        if (ix < 0x31800000)
            return x;
        return x + x * x * (0.16666666666666666 + x * x * (-0.008333333333333333 + x * x * (0.00019841269841269841 + x * x * (-2.7557319223985893e-6 + x * x * 2.4761130247707545e-8))));
    }
    if (ix > 0x41b00000) {
        w = log(fabs(x)) + 0.693147180559945309417232121458176568;
    } else if (ix > 0x40000000) {
        t = fabs(x);
        w = log(2.0 * t + 1.0 / (t + sqrt(1.0 + t * t)));
    } else {
        t = x * x;
        w = log1p(fabs(x) + t / (1.0 + sqrt(1.0 + t)));
    }
    return hx > 0 ? w : -w;
}
