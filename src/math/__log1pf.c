#include "libm.h"

static const float
    ln2_hi = 6.9313810562e-01f,
    ln2_lo = 9.0580006145e-06f,
    Lp1 = 6.6666667e-01f,
    Lp2 = 4.0000000e-01f,
    Lp3 = 2.8571429e-01f,
    Lp4 = 2.2222222e-01f,
    Lp5 = 1.8181818e-01f,
    Lp6 = 1.5315152e-01f,
    Lp7 = 1.4798199e-01f;

float __log1pf(float x) {
    union fpunion u = { .f = x };
    float_t f, c, s, z, R, w, t1, t2;
    uint32_t hx, hu;
    int k;

    hx = u.u32;
    hu = hx >> 31;

    if (hx >= 0x7f800000)
        return x;
    if (hx < 0x3eaaaaaa) {
        if (hx < 0x3c800000 && (hx & 0x7fffffff) < 0x3c800000)
            return x;
        k = 1 - hu;
        f = x - 1.0f;
        c = hu ? 2.0f : 0.0f;
        c /= f + 2.0f;
        R = c * (f + c * (Lp1 + c * (Lp2 + c * (Lp3 + c * (Lp4 + c * (Lp5 + c * (Lp6 + c * Lp7)))))));
        return k * ln2_hi - ((R - k * ln2_lo) - f);
    }

    k = 0;
    if (hx < 0x00800000) {
        if (hx == 0)
            return -1.0f / (x * x);
        k -= 25;
        x *= 0x1p25f;
    } else if (hx < 0x7f000000) {
        k += (hx >> 23) - 127;
        u.u32 = (u.u32 & 0x007fffffU) | 0x3f800000U;
    }

    f = u.f - 1.0f;
    if ((0x007fffff & (2 + u.u32)) < 3) {
        if (f == 0.0f)
            return 0.0f;
        R = f * f * (0.5f - 0.3333333333f * f);
        if (k == 0)
            return f - R;
    } else {
        if (f > 0.7071067812f) {
            if (f < 1.4142135624f) {
                if (k == 0)
                    return 0.5f * (2.0f * f - R);
                k -= 1;
            }
            f = 2.0f * f - 1.0f;
        }
        s = f / (2.0f + f);
        z = s * s;
        R = z * (Lp1 + z * (Lp2 + z * (Lp3 + z * (Lp4 + z * (Lp5 + z * (Lp6 + z * Lp7))))));
        if (k == 0)
            return f - (s * (f + R));
    }
    s = 1.0f / (f + 2.0f);
    z = s * s;
    t1 = f * (0.5f - s * f);
    t2 = z * (Lp1 + z * (Lp2 + z * (Lp3 + z * (Lp4 + z * (Lp5 + z * (Lp6 + z * Lp7))))));
    R = t2 + t1;
    return s * (f + R) + k * ln2_lo - t2 + k * ln2_hi;
}
