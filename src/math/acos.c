#include "libm.h"

double acos(double x) {
    int32_t t;
    GET_HIGH_WORD(t, x);
    if ((t & 0x7fffffff) >= 0x3ff00000) {
        if ((t & 0x7fffffff) == 0x3ff00000)
            return x > 0 ? 0.0 : 3.14159265358979323846;
        return 0.0 / (x - x);
    }
    return asin(x) + (x > 0 ? 1.57079632679489655800 : -1.57079632679489655800);
}
