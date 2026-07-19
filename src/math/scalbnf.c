#include "libm.h"

float scalbnf(float x, int n) {
    return ldexpf(x, n);
}
