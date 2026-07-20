#include "libm.h"

int __signbitl(long double x) {
    union {
        long double ld;
        unsigned char bytes[sizeof(long double)];
    } u;
    u.ld = x;

    if (sizeof(long double) == 12 || sizeof(long double) == 16) {
        return (u.bytes[9] & 0x80) != 0;
    } else {
        return (u.bytes[sizeof(long double) - 1] & 0x80) != 0;
    }
}
