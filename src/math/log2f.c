#include "libm.h"

float log2f(float x) {
    if (x <= 0)
        return -1.0f / (x - x);
    return __logf(x) * 1.4426950408889634f;
}
