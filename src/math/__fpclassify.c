#include <math.h>
#include "libm.h"

int __fpclassifyd(double x) {
    return __ifpclassify(x);
}

int __fpclassifyf(float x) {
    return __ifpclassifyf(x);
}
