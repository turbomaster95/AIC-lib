#include "libm.h"

extern void sincos(double x, double *sin, double *cos);

void sincosf(float x, float *sin, float *cos) {
    double s, c;
    sincos((double)x, &s, &c);
    *sin = (float)s;
    *cos = (float)c;
}
