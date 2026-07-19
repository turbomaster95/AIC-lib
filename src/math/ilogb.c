#include "libm.h"

int ilogb(double x) {
    union fpunion u = { .d = x };
    int e = u.u64 >> 52 & 0x7ff;

    if (!e) {
        if (u.u64 << 1 == 0)
            return FP_ILOGB0;
        do {
            x *= 0x1p64;
            e -= 64;
        } while (u.u64 >> 52 == 0);
        e += u.u64 >> 52 & 0x7ff;
        return e - 0x3ff - 1023;
    }
    if (e == 0x7ff)
        return FP_ILOGBNAN;
    return e - 0x3ff;
}
