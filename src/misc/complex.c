#include <complex.h>
#include <math.h>

double creal(double complex z) {
    return __builtin_creal(z);
}

double cimag(double complex z) {
    return __builtin_cimag(z);
}

double complex conj(double complex z) {
    return __builtin_conj(z);
}

double cabs(double complex z) {
    return hypot(creal(z), cimag(z));
}

double carg(double complex z) {
    return atan2(cimag(z), creal(z));
}

float crealf(float complex z) {
    return __builtin_crealf(z);
}

float cimagf(float complex z) {
    return __builtin_cimagf(z);
}

float complex conjf(float complex z) {
    return __builtin_conjf(z);
}

float cabsf(float complex z) {
    return hypotf(crealf(z), cimagf(z));
}
