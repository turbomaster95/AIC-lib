#include "libm.h"

double ldexp(double x, int n) {
    if (n > 1023) {
        x *= 0x1p1023;
        n -= 1023;
        if (n > 1023)
            n = 1023;
    } else if (n < -1022) {
        x *= 0x1p-1022;
        n += 1022;
        if (n < -1022)
            n = -1022;
    }
    union fpunion u;
    INSERT_WORDS(u.d, (0x3ff + n) << 20, 0);
    return x * u.d;
}
