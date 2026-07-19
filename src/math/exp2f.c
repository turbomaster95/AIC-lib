#include "libm.h"

float exp2f(float x) {
    static const float
        p1 = 1.536629169054823e-16f,
        p2 = 7.095130624770327e-03f,
        p3 = 1.151377619388076e-01f,
        p4 = 4.954565879942973e-01f,
        p5 = 1.000000000000000e+00f;

    union fpunion u = { .f = x };
    unsigned e = u.u32 >> 23 & 0xff;
    float r, t, z;

    if (e >= 0x80 || u.u32 >> 31) {
        if (isnan(x)) return x;
        if (x < -126) {
            if (x < -150) return 0;
            if (x < -126 - 24) return 0x1p-126f * 0x1p-126f;
        }
        if (x > 127) return x * 0x1p127f;
    }

    t = x + 0.5f;
    e = (int)t;
    r = x - e;
    r += (t - e) - 0.5f;

    z = r * r;
    t = z * (p1 + z * (p2 + z * (p3 + z * (p4 + z * p5))));
    t = r + r * t;

    u.u32 = (0x7f + e) << 23;
    return u.f * (1 + t);
}
