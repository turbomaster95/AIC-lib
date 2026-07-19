#include "libm.h"

double __expo2(double x, double scale) {
    static const double
        k     = 0.6931471805599453,
        hi    = 1.4426950408889634,
        lo    = 1.6751713164886512e-10;
    union fpunion u = { .d = scale };
    uint64_t hx = u.u64 & 0x7fffffffffffffffULL;
    double_t z, r, r2, y, s;

    double_t t = x * hi;
    int i0 = (int)t;
    double_t dk = (double)i0;
    r = x - dk * lo - dk * k;

    r2 = r * r;
    s = 1 + r2 * (0.16666666666666666 + r2 * (0.008333333333333333 +
         r2 * (0.00019841269841269841 + r2 * 2.7557319223985893e-6)));
    s = 1 + r + r2 * 0.5 + r2 * r2 * s;

    INSERT_WORDS(y, (hx >> 20) + ((uint64_t)i0 << 52), hx & 0xfffffU);
    return y * s;
}
