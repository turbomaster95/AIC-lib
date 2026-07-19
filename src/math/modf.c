#include "libm.h"

double modf(double x, double *iptr) {
    union fpunion u = { .d = x };
    uint64_t mask;
    int e = u.u64 >> 52 & 0x7ff;

    if (e >= 0x3ff + 52) {
        *iptr = x;
        if (e == 0x7ff && (u.u64 << 12))
            return x - x;
        u.u64 &= 0x8000000000000000ULL;
        return u.d;
    }
    if (e < 0x3ff) {
        u.u64 &= 0x8000000000000000ULL;
        *iptr = u.d;
        return x;
    }
    mask = 0x000fffffffffffffULL >> (e - 0x3ff);
    if ((u.u64 & mask) == 0) {
        *iptr = x;
        u.u64 &= 0x8000000000000000ULL;
        return u.d;
    }
    *iptr = x;
    u.u64 &= mask;
    return u.d;
}
