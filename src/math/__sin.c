#include "libm.h"

double __sin(double x, double y, int iy) {
    static const double
        S1 = -1.66666666666666324348e-01,
        S2 = 8.33333333332248946124e-03,
        S3 = -1.98412698298579493134e-04,
        S4 = 2.75573137070700676789e-06,
        S5 = -2.50507602533433802852e-08,
        S6 = 1.58969099521155010221e-10;
    double z, r, v;
    z = x * x;
    v = z * z;
    if (iy == 0)
        return x + (z * (S1 + v * (S2 + v * (S3 + v * (S4 + v * (S5 + v * S6))))));
    else
        return x - ((z * (0.5 - y) - x) + z * (S1 + v * (S2 + v * (S3 + v * (S4 + v * (S5 + v * S6))))));
}
