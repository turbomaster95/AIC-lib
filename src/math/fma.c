#include "libm.h"

#if !defined(__FMA__) && !defined(__FMA4__)
double fma(double x, double y, double z) {
    if (isnan(x) || isnan(y) || isnan(z))
        return x + y + z;
    if (x == 0 || y == 0)
        return z;
    if (isinf(x) || isinf(y)) {
        if (z == 0)
            return x * y;
        if (isinf(z))
            return z;
        return x * y;
    }

    int ex = ilogb(fabs(x));
    int ey = ilogb(fabs(y));
    if (ex + ey > 1023) {
        return ldexp(ldexp(x, -600) * ldexp(y, -600) + ldexp(z, -1200), 1200);
    }
    if (ex + ey < -1074) {
        return ldexp(ldexp(x, 600) * ldexp(y, 600) + ldexp(z, 1200), -1200);
    }
    return x * y + z;
}
#endif
