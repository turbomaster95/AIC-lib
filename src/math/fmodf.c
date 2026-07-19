#include "libm.h"

float fmodf(float x, float y) {
    union fpunion ux = { .f = x };
    union fpunion uy = { .f = y };
    int ex = ux.u32 >> 23 & 0xff;
    int ey = uy.u32 >> 23 & 0xff;
    int sx = ux.u32 >> 31;
    uint32_t i;

    if (uy.u32 << 1 == 0 || isnan(y) || ex == 0xff)
        return (x * y) / (x * y);
    if (ux.u32 << 1 <= uy.u32 << 1) {
        if (ux.u32 << 1 == uy.u32 << 1)
            return 0 * x;
        return x;
    }

    if (!ex) {
        for (i = ux.u32 << 9; i >> 31 == 0; ex--, i <<= 1);
        ux.u32 <<= -ex + 1;
    } else {
        ux.u32 &= 0x007fffffU;
        ux.u32 |= 0x00800000U;
    }
    if (!ey) {
        for (i = uy.u32 << 9; i >> 31 == 0; ey--, i <<= 1);
        uy.u32 <<= -ey + 1;
    } else {
        uy.u32 &= 0x007fffffU;
        uy.u32 |= 0x00800000U;
    }

    while (ex > ey) {
        i = ux.u32 - uy.u32;
        if (i >> 31 == 0) {
            if (i == 0)
                return 0 * x;
            ux.u32 = i;
        }
        ux.u32 <<= 1;
        ex--;
    }
    i = ux.u32 - uy.u32;
    if (i >> 31 == 0) {
        if (i == 0)
            return 0 * x;
        ux.u32 = i;
    }
    while (ux.u32 >> 23 == 0)
        ux.u32 <<= 1;

    if (ex > 0) {
        ux.u32 -= 0x00800000U;
        ux.u32 |= (uint32_t)(ex - 1) << 23;
    } else {
        ux.u32 -= 0x00800000U;
        ux.u32 |= (uint32_t)(1 - ex) << 23;
        ux.u32 = -(int32_t)ux.u32;
    }
    ux.u32 |= (uint32_t)sx << 31;
    return ux.f;
}
