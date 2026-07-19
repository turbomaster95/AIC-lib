#include "libm.h"

static const double
    ln2_hi = 6.93147180369123816490e-01,
    ln2_lo = 1.90821492927058770002e-10,
    Lp1 = 6.666666666666735130e-01,
    Lp2 = 3.999999999940941908e-01,
    Lp3 = 2.857142874366239149e-01,
    Lp4 = 2.222219843214978396e-01,
    Lp5 = 1.818357216161805012e-01,
    Lp6 = 1.531383769920937332e-01,
    Lp7 = 1.479819860511658591e-01;

double __log1p(double x) {
    union fpunion u = { .d = x };
    double_t hfsq, f, c, s, z, R, w, t1, t2;
    uint32_t hx, hu;
    int k;

    hx = u.u64 >> 32;
    hu = hx >> 31;

    if (hx >= 0x7ff00000)
        return x;
    if (hx < 0x3eaaaaaa) {
        if (hx < 0x3c800000 && (hx & 0x7fffffff) < 0x3c800000)
            return x;
        k = 1 - hu;
        f = x - 1.0;
        if (hu == 0)
            c = 0;
        else
            c = 2.0;
        c /= f + 2.0;
        R = c * (f + c * (Lp1 + c * (Lp2 + c * (Lp3 + c * (Lp4 + c * (Lp5 + c * (Lp6 + c * Lp7)))))));
        return k * ln2_hi - ((R - k * ln2_lo) - f);
    }

    k = 0;
    if (hx < 0x00100000) {
        if (hx == 0)
            return -1.0 / (x * x);
        k -= 54;
        x *= 0x1p54;
    } else if (hx < 0x7fe00000) {
        k += (hx >> 20) - 0x3ff;
        u.u64 = (u.u64 & 0x000fffffffffffffULL) | 0x3ff0000000000000ULL;
    }

    f = u.d - 1.0;
    if ((0x000fffff & (2 + (u.u64 >> 32))) < 3) {
        if (f == 0.0)
            return 0.0;
        R = f * f * (0.5 - 0.33333333333333333 * f);
        if (k == 0)
            return f - R;
    } else {
        if (f > 0.70710678118654752) {
            if (f < 1.4142135623730951) {
                if (k == 0)
                    return 0.5 * (2.0 * f - R);
                k -= 1;
            }
            f = 2.0 * f - 1.0;
        }
        s = f / (2.0 + f);
        z = s * s;
        R = z * (Lp1 + z * (Lp2 + z * (Lp3 + z * (Lp4 + z * (Lp5 + z * (Lp6 + z * Lp7))))));
        if (k == 0)
            return f - (s * (f + R));
    }
    s = 1.0 / (f + 2.0);
    z = s * s;
    t1 = f * (0.5 - s * f);
    t2 = z * (Lp1 + z * (Lp2 + z * (Lp3 + z * (Lp4 + z * (Lp5 + z * (Lp6 + z * Lp7))))));
    R = t2 + t1;
    return s * (f + R) + k * ln2_lo - t2 + k * ln2_hi;
}
