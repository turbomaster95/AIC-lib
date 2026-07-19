#include "libm.h"

static const uint32_t
    B1 = 715094163,
    B2 = 696219795;

static const double
    C = 5.42857142857142815906e-01,
    D = -7.05306122448979195096e-01,
    E = 1.41428571428571436819e+00,
    F = 1.60714285714285720630e+00,
    G = 3.57142857142857150787e-01;

double cbrt(double x) {
    int32_t hx;
    double_t t, r, s, w, sign;

    GET_HIGH_WORD(hx, x);
    sign = 1.0;
    if (hx < 0) {
        sign = -1.0;
        hx &= 0x7fffffff;
    }
    if (hx >= 0x7ff00000)
        return x + x;

    if (hx < 0x00100000) {
        if (hx == 0)
            return x;
        return sign * cbrt(x * 0x1p54) * 0x1p-18;
    }

    INSERT_WORDS(t, hx / 3 + B2, 0);

    r = x / (t * t);
    r = t * t * r + r;
    s = t * (C + D * r) + E;
    r = x / (s * s);
    r = s * s * r + r;
    w = t + t * (r - t) / (F + G * r);

    return sign * w;
}
