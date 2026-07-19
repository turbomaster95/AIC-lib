#include "libm.h"

double sin(double x) {
    double y[2];
    int32_t n, ix;

    GET_HIGH_WORD(ix, x);
    ix &= 0x7fffffff;

    if (ix <= 0x3fe921fb) {
        if (ix < 0x3e500000) {
            if ((int)x == 0)
                return x;
        }
        return __sin(x, 0.0, 0);
    }

    if (ix >= 0x7ff00000)
        return x - x;

    n = __rem_pio2(x, y);
    switch (n & 3) {
    case 0: return  __sin(y[0], y[1], 0);
    case 1: return  __cos(y[0], y[1]);
    case 2: return -__sin(y[0], y[1], 0);
    case 3: return -__cos(y[0], y[1]);
    }
    return 0;
}
