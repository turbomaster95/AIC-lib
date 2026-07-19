#include "libm.h"

static const double one = 1.0;

static const double
    pio2_hi = 1.57079632679489655800e+00,
    pio2_lo = 6.12323399573676603587e-17,
    pS0 = 1.66666666666666657415e-01,
    pS1 = -3.25565818622400915405e-01,
    pS2 = 2.01212532134862925881e-01,
    pS3 = -4.00555345006794114027e-02,
    pS4 = 7.91534994289814532176e-04,
    pS5 = 3.47933107596021167570e-05,
    qS1 = -2.40339491173441421878e+00,
    qS2 = 2.02094576023350569471e+00,
    qS3 = -6.88283971605453293030e-01,
    qS4 = 7.70381505559019352791e-02;

double asin(double x) {
    double_t t, p, q, c, r, s;
    int32_t hx, ix;

    GET_HIGH_WORD(hx, x);
    ix = hx & 0x7fffffff;
    if (ix >= 0x3ff00000) {
        uint32_t lx;
        GET_LOW_WORD(lx, x);
        if (((ix - 0x3ff00000) | lx) == 0)
            return x * pio2_hi + 0x0;
        return (x - x) / (x - x);
    }
    if (ix < 0x3fe00000) {
        if (ix < 0x3e400000) {
            if (ix < 0x31800000)
                return x;
            return x + x * x * (pS0 + x * (pS1 + x * (pS2 + x * (pS3 + x * (pS4 + x * pS5)))));
        }
        t = x * x;
        p = t * (pS0 + t * (pS1 + t * (pS2 + t * (pS3 + t * (pS4 + t * pS5)))));
        q = one + t * (qS1 + t * (qS2 + t * (qS3 + t * qS4)));
        return x + x * p / q;
    }
    if (hx > 0)
        x = -x;
    t = (one - x) * 0.5;
    p = t * (pS0 + t * (pS1 + t * (pS2 + t * (pS3 + t * (pS4 + t * pS5)))));
    q = one + t * (qS1 + t * (qS2 + t * (qS3 + t * qS4)));
    s = sqrt(t);
    if (ix >= 0x3FEF3333)
        r = pio2_hi - 2.0 * (s + s * (p / q));
    else
        r = pio2_hi - (2.0 * s - pio2_lo + s * p / q);
    if (hx > 0)
        r = -r;
    return r;
}

