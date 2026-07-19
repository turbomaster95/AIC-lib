#include "libm.h"

static const float toint = 1.0f / FLT_EPSILON;

float rintf(float x) {
    union fpunion u = { .f = x };
    int e = u.u32 >> 23 & 0xff;
    float_t y;

    if (e >= 0x7f + 23)
        return x;
    if (u.u32 >> 31)
        y = x - toint + toint - x;
    else
        y = x + toint - toint - x;
    if (e <= 0x7f - 1) {
        FORCE_EVAL(y);
        return 0.0f;
    }
    return x + y;
}

