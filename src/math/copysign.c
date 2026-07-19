#include "libm.h"

double copysign(double x, double y) {
    union fpunion ux = { .d = x };
    union fpunion uy = { .d = y };
    ux.u64 = (ux.u64 & 0x7fffffffffffffffULL) | (uy.u64 & 0x8000000000000000ULL);
    return ux.d;
}
