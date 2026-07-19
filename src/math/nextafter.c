#include "libm.h"

double nextafter(double x, double y) {
    union fpunion ux = { .d = x };
    union fpunion uy = { .d = y };
    uint64_t ax, ay;

    if (isnan(x) || isnan(y))
        return x + y;
    if (ux.u64 == uy.u64)
        return y;
    ax = ux.u64 & 0x7fffffffffffffffULL;
    ay = uy.u64 & 0x7fffffffffffffffULL;
    if (ax == 0) {
        if (ay == 0)
            return y;
        ux.u64 = 1;
        if (ux.u64 >> 63 != uy.u64 >> 63)
            ux.u64 |= 0x8000000000000000ULL;
        return ux.d;
    }
    if (ax > ay || ((ax == ay) && (ux.u64 > uy.u64))) {
        if (ux.u64 >> 63 == 0)
            ux.u64--;
        else
            ux.u64++;
    } else {
        if (ux.u64 >> 63 == 0)
            ux.u64++;
        else
            ux.u64--;
    }
    return ux.d;
}
