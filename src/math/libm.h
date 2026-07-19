#ifndef LIBM_H
#define LIBM_H

#include <stdint.h>
#include <float.h>
#include <math.h>
#include <bits/types.h>

/* Union for manipulating floating point types */
union fpunion {
    float f;
    double d;
    long double ld;
    uint32_t u32;
    uint64_t u64;
};

/* Double precision bit manipulation */
#define GET_HIGH_WORD(hi, _val_) \
    do { union fpunion _u; _u.d = (_val_); (hi) = _u.u64 >> 32; } while(0)

#define GET_LOW_WORD(lo, _val_) \
    do { union fpunion _u; _u.d = (_val_); (lo) = (uint32_t)_u.u64; } while(0)

#define SET_HIGH_WORD(_val_, hi) \
    do { union fpunion _u; _u.u64 = ((uint64_t)(hi) << 32) | ((uint32_t)_u.u64); (_val_) = _u.d; } while(0)

#define SET_LOW_WORD(_val_, lo) \
    do { union fpunion _u; _u.u64 = (_u.u64 & 0xFFFFFFFF00000000ULL) | (uint32_t)(lo); (_val_) = _u.d; } while(0)

#define INSERT_WORDS(_val_, hi, lo) \
    do { union fpunion _u; _u.u64 = ((uint64_t)(hi) << 32) | (uint32_t)(lo); (_val_) = _u.d; } while(0)

#define EXTRACT_WORDS(hi, lo, _val_) \
    do { union fpunion _u; _u.d = (_val_); (hi) = _u.u64 >> 32; (lo) = (uint32_t)_u.u64; } while(0)

/* Float precision bit manipulation */
#define GET_FLOAT_WORD(w, _fval_) \
    do { union fpunion _u; _u.f = (_fval_); (w) = _u.u32; } while(0)

#define SET_FLOAT_WORD(_fval_, w) \
    do { union fpunion _u; _u.u32 = (w); (_fval_) = _u.f; } while(0)

/* Constants */
#define PINF   INFINITY
#define NINF   (-INFINITY)
#define NAN    (__builtin_nan(""))
#define PI     3.14159265358979323846264338327950288
#define PIO2   1.57079632679489661923132169163975144
#define PIO4   0.785398163397448309615660845819875721

#define PIo2_1  1.57079625129699707031e+00  /* 0x3FF921FB40000000 */
#define PIo2_1t 7.54978941586159635335e-08  /* 0x3E74442D00000000 */
#define PIo2_2  7.54978941586159635335e-08  /* 0x3E74442D00000000 */
#define PIo2_2t 5.39012080372195280713e-15  /* 0x3DF85E45835EDA94 */
#define PIo2_3  5.39012080372195280713e-15  /* 0x3DF85E45835EDA94 */
#define PIo2_3t 2.85683300795049655243e-22  /* 0x3C867E48B7AFC855 */

/* Overflow/threshold values */
#define DBL_MAX_EXP 1024
#define FLT_MAX_EXP 128
#define DBL_MAX_LOG 0x7fefffffffffffffULL
#define FLT_MAX_LOG 0x7f7fffffU

/* Helper functions */
int32_t __rem_pio2(double x, double *y);
int32_t __rem_pio2f(float x, double *y);
int32_t __rem_pio2l(long double x, long double *y);
double __sin(double x, double y, int iy);
double __cos(double x, double y);
double __tan(double x, double y, int iy);
double __expo2(double x, double scale);
float __expf(float x);
double __log(double x);
float __log1pf(float x);
float __logf(float x);
double __log1p(double x);

/* Force evaluation for strict FP */
#define FORCE_EVAL(x) do { if (sizeof(x) == sizeof(float)) { \
    volatile float __x; __x = (x); \
} else if (sizeof(x) == sizeof(double)) { \
    volatile double __x; __x = (x); \
} else { \
    volatile long double __x; __x = (x); \
} } while(0)

static inline int __ifpclassify(double x) {
    union fpunion u = { .d = x };
    int e = u.u64 >> 52 & 0x7ff;
    if (!e) return u.u64 << 1 ? FP_SUBNORMAL : FP_ZERO;
    if (e == 0x7ff) return u.u64 << 12 ? FP_NAN : FP_INFINITE;
    return FP_NORMAL;
}

static inline int __ifpclassifyf(float x) {
    union fpunion u = { .f = x };
    int e = u.u32 >> 23 & 0xff;
    if (!e) return u.u32 << 1 ? FP_SUBNORMAL : FP_ZERO;
    if (e == 0xff) return u.u32 << 9 ? FP_NAN : FP_INFINITE;
    return FP_NORMAL;
}

/* Polynomial evaluation helpers */
static inline double __poly1(double x, double c0, double c1) {
    return c0 + c1 * x;
}

static inline double __poly2(double x, double c0, double c1, double c2) {
    return c0 + (c1 + c2 * x) * x;
}

static inline double __poly3(double x, double c0, double c1, double c2, double c3) {
    return c0 + (c1 + (c2 + c3 * x) * x) * x;
}

static inline double __poly4(double x, double c0, double c1, double c2, double c3, double c4) {
    return c0 + (c1 + (c2 + (c3 + c4 * x) * x) * x) * x;
}

static inline double __poly5(double x, double c0, double c1, double c2, double c3, double c4, double c5) {
    return c0 + (c1 + (c2 + (c3 + (c4 + c5 * x) * x) * x) * x) * x;
}

#endif /* LIBM_H */
