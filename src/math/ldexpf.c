#include "libm.h"

float ldexpf(float x, int n) {
    if (n > 127) {
        x *= 0x1p127f;
        n -= 127;
        if (n > 127)
            n = 127;
    } else if (n < -126) {
        x *= 0x1p-126f;
        n += 126;
        if (n < -126)
            n = -126;
    }
    union fpunion u;
    u.u32 = (0x7f + n) << 23;
    return x * u.f;
}
