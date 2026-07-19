#include "libm.h"

float log1pf(float x) {
    if (x <= -1.0f)
        return -1.0f / (x + 1.0f);
    return __log1pf(x);
}
