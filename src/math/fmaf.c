#include "libm.h"

#if !defined(__FMA__) && !defined(__FMA4__)
float fmaf(float x, float y, float z) {
    return (float)fma((double)x, (double)y, (double)z);
}
#endif
