#include "libm.h"

float fdimf(float x, float y) {
    if (isnan(x) || isnan(y))
        return NAN;
    return x > y ? x - y : 0.0f;
}
