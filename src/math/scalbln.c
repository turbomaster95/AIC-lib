#include "libm.h"

double scalbln(double x, long n) {
    if (n > 1023)
        n = 1023;
    if (n < -1022)
        n = -1022;
    return ldexp(x, n);
}
