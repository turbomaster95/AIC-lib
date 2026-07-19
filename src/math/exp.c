#include "libm.h"

static const double
    ln2hi  = 6.93147180369123816490e-01,  /* 0x3fe62e42, 0xfefa3800 */
    ln2lo  = 1.90821492927058770002e-10,  /* 0x3dea39ef, 0x35793c76 */
    invln2 = 1.44269504088896338700e+00,  /* 0x3ff71547, 0x652b82fe */
    P1     = 1.66666666666666019037e-01,  /* 0x3FC55555, 0x5555553E */
    P2     = -2.77777777770155933842e-03, /* 0xBF66C16C, 0x16BEBD93 */
    P3     = 6.61375632143793436117e-05,  /* 0x3F11566A, 0xAF25DE2C */
    P4     = -1.65339022054652515390e-06, /* 0xBEBBBD41, 0xC5D26BF1 */
    P5     = 4.13813679705723846039e-08;  /* 0x3E663769, 0x72BEA4D0 */

double exp(double x) {
    double_t hi, lo, c, t, y, z;
    int32_t k, sign;
    uint32_t hx;

    GET_HIGH_WORD(hx, x);
    sign = hx >> 31;
    hx &= 0x7fffffff;

    if (hx >= 0x40862e42) {
        if (isnan(x)) return x;
        if (x > 709.782712893383973096) {
            /* overflow */
            x *= 0x1p1023;
            return x;
        }
        if (x < -708.39641853226410622) {
            /* underflow */
            if (x < -745.13321910194110842)
                return 0;
            return 0x1p-1022 * 0x1p-1022;
        }
    }

    if (hx > 0x3fd62e42) {
        if (hx < 0x3FF0A2B2) {
            if (sign) {
                hi = x - ln2hi;
                lo = ln2lo;
                k = -1;
            } else {
                hi = x + ln2hi;
                lo = -ln2lo;
                k = 1;
            }
        } else {
            k = (int)(invln2 * x + (sign ? -0.5 : 0.5));
            t = k;
            hi = x - t * ln2hi;
            lo = t * ln2lo;
        }
        x = hi - lo;
        c = (hi - x) - lo;
    } else if (hx > 0x3e300000) {
        k = 0;
        c = 0;
        if (hx < 0x3c900000)
            FORCE_EVAL(0x1p1023 + x);
    } else {
        return 1 + x;
    }

    t = x * x;
    c += x * t * (P1 + t * (P2 + t * (P3 + t * (P4 + t * P5))));
    y = 1 + (x * c + (x * x) * 0.5);
    if (k == 0)
        return y;

    INSERT_WORDS(t, 0x3ff00000 + (k << 20), 0);
    return y * t;
}
