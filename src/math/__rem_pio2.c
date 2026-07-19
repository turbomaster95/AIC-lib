#include "libm.h"

static const double
    pio2_1  = 1.57079625129699707031e+00,
    pio2_1t = 7.54978941586159635335e-08,
    pio2_2  = 7.54978941586159635335e-08,
    pio2_2t = 5.39012080372195280713e-15,
    pio2_3  = 5.39012080372195280713e-15,
    pio2_3t = 2.85683300795049655243e-22,
    invpio2 = 6.36619772367581382433e-01;

int32_t __rem_pio2(double x, double *y) {
    int32_t n, tx, ix;
    uint32_t lx;
    EXTRACT_WORDS(tx, lx, x);
    ix = tx & 0x7fffffff;

    if (ix <= 0x3fe921fb) {
        if (ix < 0x3e400000) {
            if ((int)x == 0)
                return 0;
        }
        y[0] = x;
        y[1] = 0;
        return 0;
    }

    if (ix < 0x433921fb) {
        double t;
        if (ix < 0x41600000) {
            t = fabs(x);
            n = (int32_t)(t * invpio2 + 0.5);
            y[0] = t - n * pio2_1;
            y[1] = t - n * pio2_1 - y[0];
        } else {
            double w;
            n = 0;
            while (ix >= 0x401921fb) {
                ix -= 0x00100000;
                w = (double)(0x00100000 | (lx >> 31));
                t = x - w;
                EXTRACT_WORDS(tx, lx, t);
                ix = tx & 0x7fffffff;
                n++;
            }
            w = (double)((0x00100000 | (lx >> 31)) - (ix >> 20));
            t = x - w;
            EXTRACT_WORDS(tx, lx, t);
            ix = tx & 0x7fffffff;
            n += (ix >> 20);
            y[0] = t - n * pio2_1;
            y[1] = t - n * pio2_1 - y[0];
        }
        if (tx < 0) {
            y[0] = -y[0];
            y[1] = -y[1];
            return -n;
        }
        return n;
    }

    double r = fmod(x, PIO2);
    n = (int32_t)((x - r) / PIO2);
    y[0] = r;
    y[1] = 0.0;
    return n;
}
