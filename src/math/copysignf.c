#include "libm.h"

float copysignf(float x, float y) {
    union fpunion ux = { .f = x };
    union fpunion uy = { .f = y };
    ux.u32 = (ux.u32 & 0x7fffffffU) | (uy.u32 & 0x80000000U);
    return ux.f;
}
