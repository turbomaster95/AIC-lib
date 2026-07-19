#include "libm.h"

double log(double x) {
    if (x <= 0)
        return -1 / (x - x);
    return __log(x);
}
