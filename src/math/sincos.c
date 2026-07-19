#include "libm.h"

void sincos(double x, double *sin, double *cos) {
    double y[2];
    int32_t n, ix;

    GET_HIGH_WORD(ix, x);
    ix &= 0x7fffffff;

    if (ix <= 0x3fe921fb) {
        if (ix < 0x3e500000) {
            if ((int)x == 0) {
                *sin = x;
                *cos = 1.0;
                return;
            }
        }
        *sin = __sin(x, 0.0, 0);
        *cos = __cos(x, 0.0);
        return;
    }

    if (ix >= 0x7ff00000) {
        *sin = x - x;
        *cos = x - x;
        return;
    }

    n = __rem_pio2(x, y);
    switch (n & 3) {
    case 0:
        *sin = __sin(y[0], y[1], 0);
        *cos = __cos(y[0], y[1]);
        break;
    case 1:
        *sin = __cos(y[0], y[1]);
        *cos = -__sin(y[0], y[1], 0);
        break;
    case 2:
        *sin = -__sin(y[0], y[1], 0);
        *cos = -__cos(y[0], y[1]);
        break;
    case 3:
        *sin = -__cos(y[0], y[1]);
        *cos = __sin(y[0], y[1], 0);
        break;
    }
}
