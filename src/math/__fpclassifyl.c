#include "libm.h"

int __fpclassifyl(long double x) {
    union {
        long double ld;
        struct {
            unsigned long long mantissa;
            unsigned short exp_sign;
            unsigned short pad[3];
        } x87;
        struct {
            unsigned long long mantissa;
            unsigned short exp_sign;
        } d64;
    } u;
    u.ld = x;

    if (sizeof(long double) == 12 || sizeof(long double) == 16) {
        unsigned short exp = u.x87.exp_sign & 0x7FFF;
        unsigned long long mant = u.x87.mantissa;

        if (exp == 0) {
            return (mant == 0) ? FP_ZERO : FP_SUBNORMAL;
        }
        if (exp == 0x7FFF) {
            return (mant == 0x8000000000000000ULL) ? FP_INFINITE : FP_NAN;
        }
        return FP_NORMAL;
    } else {
        unsigned short exp = u.d64.exp_sign & 0x7FF;
        unsigned long long mant = u.d64.mantissa & 0x000FFFFFFFFFFFFFULL;

        if (exp == 0) {
            return (mant == 0) ? FP_ZERO : FP_SUBNORMAL;
        }
        if (exp == 0x7FF) {
            return (mant == 0) ? FP_INFINITE : FP_NAN;
        }
        return FP_NORMAL;
    }
}
