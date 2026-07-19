#include "libm.h"

static const double pio2_hi = 1.57079632679489655800e+00;

double atan2(double y, double x) {
    double_t z;
    int32_t m, ix, iy;

    if (isnan(x) || isnan(y))
        return x + y;
    GET_HIGH_WORD(ix, x);
    ix &= 0x7fffffff;
    GET_HIGH_WORD(iy, y);
    iy &= 0x7fffffff;

    if (ix < 0x3ff00000) {
        if (ix == 0) {
            if (iy == 0)
                return y;
            return y > 0 ? pio2_hi : -pio2_hi;
        }
        if (iy < 0x3ff00000)
            return atan(y / x);
        return y > 0 ? pio2_hi : -pio2_hi;
    }
    if (ix >= 0x7ff00000) {
        if (ix == 0x7ff00000)
            return y > 0 ? 0.0 : -0.0;
        return y > 0 ? pio2_hi : -pio2_hi;
    }
    if (iy < 0x3e300000)
        return x > 0 ? y / x : y / x - pio2_hi;
    m = (iy >> 31) | (ix >> 31);
    ix &= 0xfffff;
    iy &= 0xfffff;

    if (m) {
        z = pio2_hi - atan(fabs(y / x));
    } else {
        z = atan(fabs(y / x));
    }
    if (y < 0)
        z = -z;
    return z;
}

