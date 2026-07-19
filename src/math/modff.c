#include "libm.h"

float modff(float x, float *iptr) {
    union fpunion u = { .f = x };
    uint32_t mask;
    int e = u.u32 >> 23 & 0xff;

    if (e >= 0x7f + 23) {
        *iptr = x;
        if (e == 0xff && (u.u32 << 9))
            return x - x;
        u.u32 &= 0x80000000U;
        return u.f;
    }
    if (e < 0x7f) {
        u.u32 &= 0x80000000U;
        *iptr = u.f;
        return x;
    }
    mask = 0x007fffffU >> (e - 0x7f);
    if ((u.u32 & mask) == 0) {
        *iptr = x;
        u.u32 &= 0x80000000U;
        return u.f;
    }
    *iptr = x;
    u.u32 &= mask;
    return u.f;
}
