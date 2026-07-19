#include "libm.h"

double fabs(double x) {
    union fpunion u = { .d = x };
    u.u64 &= 0x7fffffffffffffffULL;
    return u.d;
}
