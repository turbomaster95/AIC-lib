#include "libm.h"

static const double one = 1.0;

double __cos(double x, double y) {
    static const double
        C1 = 4.16666666666666019037e-02,
        C2 = -1.38888888888741095749e-03,
        C3 = 2.48015872894767294178e-05,
        C4 = -2.75573143513906633035e-07,
        C5 = 2.08757232129817482790e-09,
        C6 = -1.13596475577881948265e-11;
    double z, w;
    z = x * x;
    w = z * z;
    return one - (0.5 * z - (z * z * (C1 + w * (C2 + w * (C3 + w * (C4 + w * (C5 + w * C6)))))));
}
