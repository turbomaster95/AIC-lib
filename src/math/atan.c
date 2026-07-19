#include "libm.h"

static const double atanhi[] = {
    4.63647609000806093515e-01,
    7.85398163397448278999e-01,
    9.82793723247329054082e-01,
    1.57079632679489655800e+00,
};

static const double atanlo[] = {
    5.01215824105546358830e-10,
    3.06161699786838301793e-17,
    1.39033110312309984518e-17,
    6.12323399573676603587e-17,
};

static const double aT[] = {
    3.33333333333329318027e-01,
    -1.99999999998764832476e-01,
    1.42857142725034663711e-01,
    -1.11111105645297480688e-01,
    9.09088713343650656769e-02,
    -7.69187620504482999495e-02,
    6.66107313738753120669e-02,
    -5.83357013379057348645e-02,
    4.97687799461593236017e-02,
    -3.65315727442169155271e-02,
    1.62858201153657823623e-02,
};

double atan(double x) {
    double_t w, s1, s2, z;
    int32_t ix, hx, sign;

    GET_HIGH_WORD(hx, x);
    ix = hx & 0x7fffffff;
    if (ix >= 0x44100000) {
        if (ix > 0x7ff00000 || (ix == 0x7ff00000 && (hx & 0x000fffff)))
            return x + x;
        if (hx > 0)
            return atanhi[3] + atanlo[3];
        else
            return -atanhi[3] - atanlo[3];
    }
    if (ix < 0x3fdc0000) {
        if (ix < 0x3e400000) {
            if (ix < 0x31800000 && (int)x == 0)
                return x;
            return x + x * aT[0];
        }
        if (ix < 0x3ff30000) {
            return x + x * (aT[0] + x * x * (aT[2] + x * x * (aT[4] + x * x * (aT[6] + x * x * (aT[8] + x * x * aT[10])))));
        }
        ix = 0;
    }
    sign = hx >> 31;
    if (ix >= 0x41d00000) {
        if (sign)
            x = -x;
        w = atanhi[3] - atanlo[3] - x;
        return sign ? -w : w;
    }
    if (sign)
        x = -x;
    z = x * x;
    w = z * (aT[1] + z * (aT[3] + z * (aT[5] + z * (aT[7] + z * aT[9]))));
    s1 = z * w;
    s2 = z * s1;
    if (ix < 0x3ff30000) {
        if (ix < 0x3fe60000)
            return x - (s1 + s2);
        else
            return x - 0.5 * s1;
    }
    if (ix < 0x40300000) {
        return (0.5 * (atanhi[1] + atanlo[1]) - x + 0.5 * w) - 0.5 * s1;
    }
    return (0.5 * (atanhi[2] + atanlo[2]) + 0.5 * w) - (x - 0.5 * s1);
}
