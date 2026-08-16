#ifndef _COMPLEX_H
#define _COMPLEX_H

#include <features.h>

#define complex _Complex
#define _Complex_I 1.0fi
#define I _Complex_I

/* Double precision complex math */
double complex cacos(double complex z);
double complex casin(double complex z);
double complex catan(double complex z);
double complex ccos(double complex z);
double complex csin(double complex z);
double complex ctan(double complex z);

double complex ccosh(double complex z);
double complex csinh(double complex z);
double complex ctanh(double complex z);

double complex cexp(double complex z);
double complex clog(double complex z);
double complex cpow(double complex x, double complex y);
double complex csqrt(double complex z);

double cabs(double complex z);
double carg(double complex z);
double creal(double complex z);
double cimag(double complex z);
double complex conj(double complex z);
double complex cproj(double complex z);

/* Float precision complex math */
float complex cacosf(float complex z);
float complex casinf(float complex z);
float complex catanf(float complex z);
float complex ccosf(float complex z);
float complex csinf(float complex z);
float complex ctanf(float complex z);

float cabsf(float complex z);
float cargf(float complex z);
float crealf(float complex z);
float cimagf(float complex z);
float complex conjf(float complex z);

#endif /* _COMPLEX_H */
