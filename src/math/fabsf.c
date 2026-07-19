#include "libm.h"

float fabsf(float x) {
    union fpunion u = { .f = x };
    u.u32 &= 0x7fffffffU;
    return u.f;
}
