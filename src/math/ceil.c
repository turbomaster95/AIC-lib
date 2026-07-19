#include "libm.h"

static const double toint = 1.0 / DBL_EPSILON;

double ceil(double x) {
    union fpunion u = { .d = x };
    int e = u.u64 >> 52 & 0x7ff;
    double_t y;

    if (e >= 0x3ff + 52 || x == 0)
        return x;
    if (u.u64 >> 63)
        y = x - toint + toint - x;
    else
        y = x + toint - toint - x;
    if (e <= 0x3ff - 1) {
        FORCE_EVAL(y);
        return u.u64 >> 63 ? -0.0 : 1.0;
    }
    if (y < 0)
        return x + y + 1;
    return x + y;
}

