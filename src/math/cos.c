#include "libm.h"

static const double one = 1.0;

double cos(double x) {
    double y[2];
    int32_t n, ix;

    GET_HIGH_WORD(ix, x);
    ix &= 0x7fffffff;

    if (ix <= 0x3fe921fb) {
        if (ix < 0x3e46a09e)
            return one;
        return __cos(x, 0.0);
    }

    if (ix >= 0x7ff00000)
        return x - x;

    n = __rem_pio2(x, y);
    switch (n & 3) {
    case 0: return  __cos(y[0], y[1]);
    case 1: return -__sin(y[0], y[1], 0);
    case 2: return -__cos(y[0], y[1]);
    case 3: return  __sin(y[0], y[1], 0);
    }
    return 0;
}
