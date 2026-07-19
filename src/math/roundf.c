#include "libm.h"

static const float toint = 1.0f / FLT_EPSILON;

float roundf(float x) {
    union fpunion u = { .f = x };
    int e = u.u32 >> 23 & 0xff;
    float_t y;

    if (e >= 0x7f + 23)
        return x;
    if (u.u32 >> 31)
        y = x - toint + toint + x - toint;
    else
        y = x + toint - toint - x + toint;
    if (e <= 0x7f - 1) {
        FORCE_EVAL(y);
        return u.u32 >> 31 ? -0.0f : 0.0f;
    }
    return y;
}

