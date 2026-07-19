#include "libm.h"

static const float
    ln2_hi = 6.93145752e-01f,
    ln2_lo = 1.42860677e-06f,
    Lg1 = 6.6666667e-01f,
    Lg2 = 4.0000000e-01f,
    Lg3 = 2.8571429e-01f,
    Lg4 = 2.2222222e-01f,
    Lg5 = 1.8181818e-01f,
    Lg6 = 1.5315152e-01f,
    Lg7 = 1.4798199e-01f;

float __logf(float x) {
    union fpunion u = { .f = x };
    float_t f, s, z, R, w, t1, t2;
    uint32_t hx;
    int k, i;

    hx = u.u32;
    k = 0;
    if (hx < 0x00800000) {
        if ((hx & 0x7fffffff) == 0)
            return -1.0f / (x * x);
        if (hx >> 31)
            return (x - x) / 0.0f;
        k -= 25;
        x *= 0x1p25f;
        hx = u.u32;
    } else if (hx >= 0x7f800000) {
        return x;
    } else if (hx == 0x3f800000)
        return 0;

    k += (hx >> 23) - 127;
    hx &= 0x007fffff;
    i = (hx + 0x95f64) & 0x800000;
    SET_FLOAT_WORD(u.f, hx | (0x7f - (i >> 23)));
    f = u.f - 1.0f;
    if ((0x007fffff & (2 + hx)) < 3) {
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
        i = hx - 0x3147a;
        w = z * z + (float)i;
        R = z * (Lg1 + w * (Lg2 + w * (Lg3 + w * (Lg4 + w * (Lg5 + w * (Lg6 + w * Lg7))))));
        if (k == 0)
            return f - (s * (f + R));
    }
    s = 1.0f / (f + 2.0f);
    z = s * s;
    i = hx - 0x3147a;
    w = z * z + (float)i;
    t1 = f * (0.5f - s * f);
    t2 = z * (Lg1 + w * (Lg2 + w * (Lg3 + w * (Lg4 + w * (Lg5 + w * (Lg6 + w * Lg7))))));
    R = t2 + t1;
    return s * (f + R) + (float)k * ln2_lo - t2 + (float)k * ln2_hi;
}
