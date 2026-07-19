#include "libm.h"

float __expf(float x) {
    static const double
        k = 0.6931471805599453,
        hi = 1.4426950408889634,
        lo = 1.6751713164886512e-10;
    float t = (float)(x * hi);
    int i0 = (int)t;
    double r = x - (double)i0 * lo - (double)i0 * k;
    double r2 = r * r;
    double s = 1 + r2 * (0.16666666666666666 + r2 * (0.008333333333333333 + 
             r2 * 0.00019841269841269841));
    s = 1 + r + r2 * 0.5 + r2 * r2 * s;
    union fpunion u;
    u.u32 = 0x3f800000U + (i0 << 23);
    return (float)(u.f * s);
}
