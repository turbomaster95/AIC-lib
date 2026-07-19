#include "libm.h"

__attribute__((weak)) float sqrtf(float x) {
    union fpunion u = { .f = x };
    float y;

    if (u.u32 == 0 || u.u32 == 0x80000000U)
        return x;

    if (u.u32 >= 0x7f800000U)
        return x;

    u.u32 = (u.u32 >> 1) + 0x1fbc0000U;
    y = u.f;

    y = 0.5f * (y + x / y);
    y = 0.5f * (y + x / y);
    y = 0.5f * (y + x / y);

    return y;
}
