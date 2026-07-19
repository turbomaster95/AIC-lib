#include "libm.h"

float logf(float x) {
    if (x <= 0)
        return -1.0f / (x - x);
    return __logf(x);
}
