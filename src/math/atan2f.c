#include "libm.h"

float atan2f(float y, float x) {
    return (float)atan2((double)y, (double)x);
}
