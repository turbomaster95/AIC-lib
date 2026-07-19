#include "libm.h"

float nextafterf(float x, float y) {
    union fpunion ux = { .f = x };
    union fpunion uy = { .f = y };
    uint32_t ax, ay;

    if (isnan(x) || isnan(y))
        return x + y;
    if (ux.u32 == uy.u32)
        return y;
    ax = ux.u32 & 0x7fffffffU;
    ay = uy.u32 & 0x7fffffffU;
    if (ax == 0) {
        if (ay == 0)
            return y;
        ux.u32 = 1;
        if (ux.u32 >> 31 != uy.u32 >> 31)
            ux.u32 |= 0x80000000U;
        return ux.f;
    }
    if (ax > ay || ((ax == ay) && (ux.u32 > uy.u32))) {
        if (ux.u32 >> 31 == 0)
            ux.u32--;
        else
            ux.u32++;
    } else {
        if (ux.u32 >> 31 == 0)
            ux.u32++;
        else
            ux.u32--;
    }
    return ux.f;
}
