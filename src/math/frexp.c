#include "libm.h"

double frexp(double x, int *e) {
    union fpunion u = { .d = x };
    int ee = u.u64 >> 52 & 0x7ff;

    if (!ee) {
        if (x) {
            x = frexp(x * 0x1p64, e);
            *e -= 64;
        } else
            *e = 0;
        return x;
    } else if (ee == 0x7ff)
        return x;

    *e = ee - 0x3fe;
    u.u64 &= 0x800fffffffffffffULL;
    u.u64 |= 0x3fe0000000000000ULL;
    return u.d;
}
