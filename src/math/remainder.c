#include "libm.h"

double remainder(double x, double y) {
    uint32_t xi, yi;
    int ex, py;
    double_t r;

    if (!isfinite(x) || isnan(y))
        return (x * y) / (x * y);
    if (y == 0)
        return NAN;

    EXTRACT_WORDS(xi, yi, y);
    py = yi >> 20;
    ex = xi >> 20;

    if (ex - py > 60) {
        r = x;
        if (r != 0) {
            r = 2 * fabs(r);
            FORCE_EVAL(r);
        }
        if (x > 0)
            r -= y * floor(x / y);
        else
            r += y * floor(-x / y);
        return r;
    }

    r = fmod(x, y);
    if (fabs(r) > 0.5 * y) {
        if (r > 0)
            r -= y;
        else
            r += y;
    }
    return r;
}
