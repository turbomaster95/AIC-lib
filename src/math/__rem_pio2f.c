#include "libm.h"

int32_t __rem_pio2f(float x, double *y) {
    return __rem_pio2((double)x, y);
}
