#include "libm.h"

double exp2(double x) {
    static const double
        p1  = 1.53662916905482301372e-16,
        p2  = 7.09513062477032736108e-03,
        p3  = 1.15137761938807578442e-01,
        p4  = 4.95456587994297291432e-01,
        p5  = 1.00000000000000000000e+00;

    union fpunion u = { .d = x };
    int e = u.u64 >> 52 & 0x7ff;
    double_t r, t, z;

    if (e >= 0x408 || u.u64 >> 63) {
        if (isnan(x)) return x;
        if (x < -1022) {
            if (x < -1075) return 0;
            if (x < -1022 - 53) return 0x1p-1022 * 0x1p-1022;
        }
        if (x > 1023) return x * 0x1p1023;
    }

    t = x + 0.5;
    e = (int)t;
    r = x - e;
    r += (t - e) - 0.5;

    z = r * r;
    t = z * (p1 + z * (p2 + z * (p3 + z * (p4 + z * p5))));
    t = r + r * t;

    u.u64 = (uint64_t)(0x3ff + e) << 52;
    return u.d * (1 + t);
}
