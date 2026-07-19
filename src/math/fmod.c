#include "libm.h"

double fmod(double x, double y) {
    union fpunion ux = { .d = x };
    union fpunion uy = { .d = y };
    int ex = ux.u64 >> 52 & 0x7ff;
    int ey = uy.u64 >> 52 & 0x7ff;
    int sx = ux.u64 >> 63;
    uint64_t i;

    if (uy.u64 << 1 == 0 || isnan(y) || ex == 0x7ff)
        return (x * y) / (x * y);
    if (ux.u64 << 1 <= uy.u64 << 1) {
        if (ux.u64 << 1 == uy.u64 << 1)
            return 0 * x;
        return x;
    }

    /* normalize x and y */
    if (!ex) {
        for (i = ux.u64 << 12; i >> 63 == 0; ex--, i <<= 1);
        ux.u64 <<= -ex + 1;
    } else {
        ux.u64 &= 0x000fffffffffffffULL;
        ux.u64 |= 0x0010000000000000ULL;
    }
    if (!ey) {
        for (i = uy.u64 << 12; i >> 63 == 0; ey--, i <<= 1);
        uy.u64 <<= -ey + 1;
    } else {
        uy.u64 &= 0x000fffffffffffffULL;
        uy.u64 |= 0x0010000000000000ULL;
    }

    /* x mod y */
    while (ex > ey) {
        i = ux.u64 - uy.u64;
        if (i >> 63 == 0) {
            if (i == 0)
                return 0 * x;
            ux.u64 = i;
        }
        ux.u64 <<= 1;
        ex--;
    }
    i = ux.u64 - uy.u64;
    if (i >> 63 == 0) {
        if (i == 0)
            return 0 * x;
        ux.u64 = i;
    }
    while (ux.u64 >> 52 == 0)
        ux.u64 <<= 1;

    /* scale result up */
    if (ex > 0) {
        ux.u64 -= 0x0010000000000000ULL;
        ux.u64 |= (uint64_t)(ex - 1) << 52;
    } else {
        ux.u64 -= 0x0010000000000000ULL;
        ux.u64 |= (uint64_t)(1 - ex) << 52;
        ux.u64 = -(int64_t)ux.u64;
    }
    ux.u64 |= (uint64_t)sx << 63;
    return ux.d;
}
