#include "libm.h"

static const double
    zero    =  0.0,
    one     =  1.0,
    two     =  2.0,
    two53   =  9007199254740992.0,
    huge    =  1.0e300,
    tiny    =  1.0e-300,
    P1      =  1.66666666666666019037e-01,
    P2      = -2.77777777770155933842e-03,
    P3      =  6.61375632143793436117e-05,
    P4      = -1.65339022054652515390e-06,
    P5      =  4.13813679705723846039e-08,
    lg2     =  6.93147180559945309417e-01,
    lg2_h   =  6.93147182464599609375e-01,
    lg2_l   = -1.90465429995776804525e-09,
    omt     =  0.041666666666666664272;

double pow(double x, double y) {
    double z, ax, z_h, z_l, p_h, p_l;
    double_t y1, t1, t2, r, s, s2, s_h, s_l, t_h, t_l;
    int32_t i, j, k, yisint, n, hx, hy, ix, iy, iy1, is;
    uint32_t lx, ly;

    EXTRACT_WORDS(hx, lx, x);
    EXTRACT_WORDS(hy, ly, y);
    ix = hx & 0x7fffffff;
    iy = hy & 0x7fffffff;

    if ((iy | ly) == 0)
        return one;
    if (hx == 0x3ff00000 && lx == 0)
        return one;
    if (ix > 0x7ff00000 || (ix == 0x7ff00000 && lx != 0) ||
        iy > 0x7ff00000 || (iy == 0x7ff00000 && ly != 0))
        return x + y;
    yisint = 0;
    if (hx < 0) {
        if (iy >= 0x43400000)
            yisint = 2;
        else if (iy >= 0x3ff00000) {
            k = (iy >> 20) - 0x3ff;
            if (k > 20) {
                j = ly >> (52 - k);
                if ((j << (52 - k)) == ly)
                    yisint = 2 - (j & 1);
            } else if (ly == 0) {
                j = iy >> (20 - k);
                if ((j << (20 - k)) == iy)
                    yisint = 2 - (j & 1);
            }
        }
    }

    if (ly == 0) {
        if (iy == 0x7ff00000) {
            if (ix == 0x3ff00000 && lx == 0)
                return one;
            if (ix > 0x3ff00000)
                return y;
            return 0.0 / 0.0;
        }
        if (iy == 0x3ff00000) {
            if (y == 2.0)
                return x * x;
            if (y == -1.0)
                return 1.0 / x;
            if (y == 0.5) {
                if (hx >= 0)
                    return sqrt(x);
            }
        }
        if (hy == 0x40000000)
            return sqrt(x);
    }

    ax = fabs(x);
    if (lx == 0) {
        if (ix == 0x7ff00000 || ix == 0 || (ix == 0x3ff00000 && ly == 0)) {
            z = ax;
            if (hy < 0)
                z = one / z;
            if (hx < 0) {
                if (((ix - 0x3ff00000) | yisint) == 0)
                    z = (z - z) / (z - z);
                else if (yisint == 1)
                    z = -z;
            }
            return z;
        }
    }

    n = 0;
    if (ix < 0x00100000) {
        ax *= two53;
        n -= 53;
        GET_HIGH_WORD(ix, ax);
    }
    n += ((ix >> 20) - 0x3ff);
    j = ix & 0x000fffff;
    ix = 0x3ff00000;
    if (j <= 0x3988e)
        k = 0;
    else if (j < 0xbb67a)
        k = 1;
    else {
        k = 0;
        if (j < 0x80000) {
            if (j < 0x40000)
                SET_HIGH_WORD(ax, ix + 0x00100000);
            else
                SET_HIGH_WORD(ax, ix + 0x00200000);
        } else {
            if (j < 0xc0000)
                SET_HIGH_WORD(ax, ix + 0x00400000);
            else
                SET_HIGH_WORD(ax, ix + 0x00600000);
        }
        n -= 1;
    }
    SET_HIGH_WORD(ax, ix + (k << 20));

    z = ax * ax;
    s = z * z;
    i = j & 0x7fff;
    r = s;
    t_h = z;
    t_l = z_l = s_l = 0.0;

    y1 = y;
    GET_HIGH_WORD(iy1, y1);
    SET_LOW_WORD(y1, 0);
    p_h = y1 * t_h;
    p_l = y1 * t_l;
    z = p_h + p_l;
    j = (ix >> 20) - 0x3ff;
    i = (j < 0) ? j + 2 : j + 1;
    GET_HIGH_WORD(iy1, y1);
    k = (iy1 >> 20) - 0x3ff;
    
    if (i + k > 22) {
        GET_HIGH_WORD(is, z);
        if (is > 0x41d00000) {
            if (is == 0x7ff00000) {
                p_l *= huge;
                p_h *= huge;
                return p_l + p_h;
            }
            if (is < 0x41e00000)
                return z;
        }
    }
    t1 = ((double)n * lg2_h + z);
    t2 = z - (t1 - (double)n * lg2_h) + (double)n * lg2_l + p_l;
    s = one;
    if ((k = (int)(t1 / lg2)) != 0) {
        SET_HIGH_WORD(s, (0x3ff00000 + (k << 20)));
    }
    r = t2 - (t1 - s * lg2);
    z = one - s - s;
    z = z * s * s * (r / lg2 + one);
    t1 = s * (one + z + omt * z * z);
    GET_HIGH_WORD(is, t1);
    SET_HIGH_WORD(t1, is + 0x00100000);
    return t1;
}
