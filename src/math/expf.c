#include "libm.h"

float expf(float x) {
    if (x > 88.72283905206835f)
        return x * 0x1p127f;
    if (x < -103.9720840454453f)
        return 0;
    return __expf(x);
}
