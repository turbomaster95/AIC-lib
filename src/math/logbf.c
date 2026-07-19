#include "libm.h"

float logbf(float x) {
    union fpunion u = { .f = x };
    int e = u.u32 >> 23 & 0xff;

    if (!e) {
        if (u.u32 << 1 == 0)
            return -1.0f / 0.0f;
        do {
            x *= 0x1p64f;
            e -= 64;
        } while (u.u32 >> 23 == 0);
        e += u.u32 >> 23 & 0xff;
        return e - 0x7f - 127;
    }
    if (e == 0xff)
        return x * x;
    return e - 0x7f;
}
