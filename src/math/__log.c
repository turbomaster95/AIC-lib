#include "libm.h"

static const double
    ln2_hi = 6.93147180369123816490e-01,
    ln2_lo = 1.90821492927058770002e-10,
    Lg1 = 6.666666666666735130e-01,
    Lg2 = 3.999999999940941908e-01,
    Lg3 = 2.857142874366239149e-01,
    Lg4 = 2.222219843214978396e-01,
    Lg5 = 1.818357216161805012e-01,
    Lg6 = 1.531383769920937332e-01,
    Lg7 = 1.479819860511658591e-01;

double __log(double x) {
    union fpunion u = { .d = x };
    double_t hfsq, f, s, z, R, w, t1, t2, dk;
    int32_t k, i, hx;

    hx = u.u64 >> 32;
    k = 0;
    if (hx < 0x00100000) {
        if ((hx & 0x7fffffff) == 0)
            return -1 / (x * x);
        if (hx >> 31)
            return (x - x) / 0.0;
        k -= 54;
        x *= 0x1p54;
        hx = u.u64 >> 32;
    } else if (hx >= 0x7ff00000) {
        return x;
    } else if (hx == 0x3ff00000 && u.u64 == 0)
        return 0;

    k += (hx >> 20) - 0x3ff;
    hx &= 0x000fffff;
    i = (hx + 0x95f64) & 0x100000;
    INSERT_WORDS(u.d, hx | (0x3ff - i) << 20, 0);
    f = u.d - 1.0;
    if ((0x000fffff & (2 + hx)) < 3) {
        if (f == 0)
            return 0;
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
        i = hx - 0x6147a;
        w = z * z + i * Lg7;
        R = z * (Lg1 + w * (Lg2 + w * (Lg3 + w * (Lg4 + w * (Lg5 + w * (Lg6 + w * Lg7))))));
        if (k == 0)
            return f - (s * (f + R));
    }
    s = 1.0 / (f + 2.0);
    z = s * s;
    i = hx - 0x6147a;
    w = z * z + i * Lg7;
    t1 = f * (0.5 - s * f);
    t2 = z * (Lg1 + w * (Lg2 + w * (Lg3 + w * (Lg4 + w * (Lg5 + w * (Lg6 + w * Lg7))))));
    R = t2 + t1;
    dk = (double)k;
    return s * (f + R) + dk * ln2_lo - t2 + dk * ln2_hi;
}
