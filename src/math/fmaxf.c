#include "libm.h"

float fmaxf(float x, float y) {
    if (isnan(x))
        return y;
    if (isnan(y))
        return x;
    return x > y ? x : y;
}
