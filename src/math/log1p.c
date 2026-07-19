#include "libm.h"

double log1p(double x) {
    if (x <= -1)
        return -1 / (x + 1);
    return __log1p(x);
}
