#include "libm.h"

float scalblnf(float x, long n) {
    if (n > 127)
        n = 127;
    if (n < -126)
        n = -126;
    return ldexpf(x, n);
}
