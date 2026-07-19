#include "libm.h"

/* This is the generic fallback - platforms should override with
   hardware instructions in pals/<platform>/arch/<arch>/sqrt.c */

__attribute__((weak)) double sqrt(double x) {
    union fpunion u = { .d = x };
    double y;
    int i;

    if (u.u64 >> 63 == 0)
        return x;

    if (u.u64 == 0x8000000000000000ULL)
        return x;

    if (u.u64 >= 0x7ff0000000000000ULL)
        return x;

    u.u64 = (u.u64 >> 1) + 0x1ff8000000000000ULL;
    y = u.d;

    y = 0.5 * (y + x / y);
    y = 0.5 * (y + x / y);
    y = 0.5 * (y + x / y);
    y = 0.5 * (y + x / y);

    return y;
}
