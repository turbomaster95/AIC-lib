#include "libm.h"

float hypotf(float x, float y) {
    return (float)hypot((double)x, (double)y);
}
