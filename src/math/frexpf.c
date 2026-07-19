#include "libm.h"

float frexpf(float x, int *e) {
    union fpunion u = { .f = x };
    int ee = u.u32 >> 23 & 0xff;

    if (!ee) {
        if (x) {
            x = frexpf(x * 0x1p64f, e);
            *e -= 64;
        } else
            *e = 0;
        return x;
    } else if (ee == 0xff)
        return x;

    *e = ee - 0x7e;
    u.u32 &= 0x807fffffU;
    u.u32 |= 0x3f000000U;
    return u.f;
}
