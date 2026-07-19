#include "libm.h"

double logb(double x) {
    union fpunion u = { .d = x };
    int e = u.u64 >> 52 & 0x7ff;

    if (!e) {
        if (u.u64 << 1 == 0)
            return -1.0 / 0.0;
        do {
            x *= 0x1p64;
            e -= 64;
        } while (u.u64 >> 52 == 0);
        e += u.u64 >> 52 & 0x7ff;
        return e - 0x3ff - 1023;
    }
    if (e == 0x7ff)
        return x * x;
    return e - 0x3ff;
}
