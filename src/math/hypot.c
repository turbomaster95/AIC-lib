#include "libm.h"

double hypot(double x, double y) {
    double_t a, b, t1, t2, w;
    int32_t j, k, hx, hy, hw;

    GET_HIGH_WORD(hx, x);
    GET_HIGH_WORD(hy, y);
    j = (hx >> 20) & 0x7ff;
    k = (hy >> 20) & 0x7ff;

    if (j < k) {
        double tmp = x; x = y; y = tmp;
        int tmp_i = j; j = k; k = tmp_i;
    }

    if (j > 0x41d) {
        if (j >= 0x7ff)
            return fabs(x) + fabs(y);
        if (k >= 0x41d) {
            w = 0.5 * (fabs(x) + fabs(y));
            return w * sqrt(0.5 * (x / w * x / w + y / w * y / w));
        }
    }

    a = x;
    b = y;
    if (k > 0) {
        if (j == k) {
            a = 1.0;
            b = y / x;
        } else if (j > k) {
            a = (double)(0x00100000 >> (j - k));
            b = y * a;
            a = x * a;
        } else {
            a = x;
            b = 1.0;
        }
    } else {
        a = x;
        b = y;
        if (k == 0)
            return fabs(a) + fabs(b);
    }

    w = sqrt(a * a + b * b);
    if (j > 0x3fd) {
        t1 = w;
        GET_HIGH_WORD(hw, t1);
        SET_HIGH_WORD(t1, hw - (j - 0x3fe) * 0x100000);
        t2 = w - t1;
        w = t1 + t2 * (t2 / (2.0 * t1));
    }
    if (j > 0) {
        GET_HIGH_WORD(hw, w);
        SET_HIGH_WORD(w, hw + (j - 0x3fe) * 0x100000);
    }
    return w;
}
