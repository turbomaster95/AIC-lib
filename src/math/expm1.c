#include "libm.h"

double expm1(double x) {
    static const double
        ln2hi = 6.93147180369123816490e-01,
        ln2lo = 1.90821492927058770002e-10,
        invln2 = 1.44269504088896338700e+00,
        P1 = 1.66666666666666019037e-01,
        P2 = -2.77777777770155933842e-03,
        P3 = 6.61375632143793436117e-05,
        P4 = -1.65339022054652515390e-06,
        P5 = 4.13813679705723846039e-08;

    double_t y, c, t, hi, lo;
    int32_t k, sign;
    uint32_t hx;

    GET_HIGH_WORD(hx, x);
    sign = hx >> 31;

    if (hx >= 0x4043687a) {
        if (hx >= 0x7ff00000) {
            if (isnan(x)) return x;
            if (sign) return -1;
            return x;
        }
        if (x > 709.782712893383973096) return x * 0x1p1023;
        if (x < -708.39641853226410622) return -1;
    }

    if (hx > 0x3fd62e42) {
        if (hx < 0x3FF0A2B2) {
            if (sign) {
                hi = x - ln2hi;
                lo = ln2lo;
                k = -1;
            } else {
                hi = x + ln2hi;
                lo = -ln2lo;
                k = 1;
            }
        } else {
            k = (int)(invln2 * x + (sign ? -0.5 : 0.5));
            t = k;
            hi = x - t * ln2hi;
            lo = t * ln2lo;
        }
        x = hi - lo;
        c = (hi - x) - lo;
        t = x * x * (P1 + x * (P2 + x * (P3 + x * (P4 + x * P5))));
        y = x - (lo - t);
        y += c;
    } else if (hx > 0x3e300000) {
        k = 0;
        t = x * x * (P1 + x * (P2 + x * (P3 + x * (P4 + x * P5))));
        y = x - t;
    } else {
        return x + 0x1p-52;
    }

    if (k == 0)
        return y;

    INSERT_WORDS(t, 0x3ff00000 + (k << 20), 0);
    return y * t + (t - 1);
}
