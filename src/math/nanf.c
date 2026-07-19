#include "libm.h"

float nanf(const char *s) {
    (void)s;
    return NAN;
}
