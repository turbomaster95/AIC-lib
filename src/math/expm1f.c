#include "libm.h"

float expm1f(float x) {
    if (x > 88.72283905206835f)
        return x * 0x1p127f;
    if (x < -103.9720840454453f)
        return -1;
    return (float)expm1(x);
}
