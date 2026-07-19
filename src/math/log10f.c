#include "libm.h"

float log10f(float x) {
    if (x <= 0)
        return -1.0f / (x - x);
    return __logf(x) * 0.4342944819032518f;
}
