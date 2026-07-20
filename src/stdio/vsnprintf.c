#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>

#ifndef VSNPRINTF_BUFFER_SIZE
#define VSNPRINTF_BUFFER_SIZE 64
#endif

#ifndef VSNPRINTF_MAX_PRECISION
#define VSNPRINTF_MAX_PRECISION 1000
#endif

#ifndef VSNPRINTF_MAX_WIDTH
#define VSNPRINTF_MAX_WIDTH 100000
#endif

typedef struct {
    char *buffer;
    size_t size;
    size_t pos;
    size_t total;
    bool overflow;
    bool error;
} vsnprintf_state_t;

typedef enum {
    FLAG_LEFT       = 0x001,   /* '-' left-align */
    FLAG_PLUS       = 0x002,   /* '+' always show sign */
    FLAG_SPACE      = 0x004,   /* ' ' space for positive */
    FLAG_ZERO       = 0x008,   /* '0' zero-pad */
    FLAG_HASH       = 0x010,   /* '#' alternate form */
    FLAG_GROUP      = 0x020,   /* ' thousands grouping */
    FLAG_UPPERCASE  = 0x040,   /* uppercase variant */
    FLAG_SIGNED     = 0x080,   /* signed conversion */
    FLAG_NEGATIVE   = 0x100,   /* value is negative */
    FLAG_PRECISION  = 0x200,   /* precision was explicitly specified */
} format_flags_t;

typedef enum {
    LENGTH_NONE,
    LENGTH_HH,      /* char */
    LENGTH_H,       /* short */
    LENGTH_L,       /* long */
    LENGTH_LL,      /* long long */
    LENGTH_J,       /* intmax_t */
    LENGTH_Z,       /* size_t */
    LENGTH_T,       /* ptrdiff_t */
    LENGTH_LDBL,    /* long double */
} length_modifier_t;

__attribute__((weak)) int __fpclassifyl(long double x) {
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

__attribute__((weak)) int __signbitl(long double x) {
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

__attribute__((weak)) long double modfl(long double x, long double *iptr) {
    int cls = __fpclassifyl(x);

    if (cls == FP_NAN) {
        *iptr = x;
        return x;
    }
    if (cls == FP_INFINITE) {
        *iptr = x;
        return __signbitl(x) ? -0.0L : 0.0L;
    }

    if (x >= 9223372036854775808.0L || x <= -9223372036854775808.0L) {
        *iptr = x;
        return __signbitl(x) ? -0.0L : 0.0L;
    }

    long long i = (long long)x;
    long double integer_part = (long double)i;

    if (i == 0 && __signbitl(x)) {
        *iptr = -0.0L;
    } else {
        *iptr = integer_part;
    }

    return x - *iptr;
}

static void output_char(vsnprintf_state_t *state, char c)
{
    state->total++;
    if (state->pos < state->size - 1) {
        if (state->buffer) {
            state->buffer[state->pos] = c;
        }
        state->pos++;
    } else {
        state->overflow = true;
    }
}

static void output_string(vsnprintf_state_t *state, const char *str, size_t len)
{
    size_t i;

    if (!str) {
        str = "(null)";
        len = 6;
    }

    for (i = 0; i < len && str[i]; i++) {
        output_char(state, str[i]);
    }
}

static void output_padding(vsnprintf_state_t *state, char pad, size_t count)
{
    size_t i;
    for (i = 0; i < count; i++) {
        output_char(state, pad);
    }
}

static int parse_format_int(const char *format, size_t *pos, int default_val)
{
    int value = 0;
    bool has_value = false;

    while (format[*pos] >= '0' && format[*pos] <= '9') {
        has_value = true;
        if (value > INT_MAX / 10) {
            value = INT_MAX;
            while (format[*pos] >= '0' && format[*pos] <= '9') {
                (*pos)++;
            }
            break;
        }
        value *= 10;
        int digit = format[*pos] - '0';
        if (value > INT_MAX - digit) {
            value = INT_MAX;
            (*pos)++;
            while (format[*pos] >= '0' && format[*pos] <= '9') {
                (*pos)++;
            }
            break;
        }
        value += digit;
        (*pos)++;
    }

    return has_value ? value : default_val;
}

static size_t uint_to_string(char *buf, size_t bufsize, uintmax_t value, 
                              int base, bool uppercase)
{
    const char *digits = uppercase ? "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   : "0123456789abcdefghijklmnopqrstuvwxyz";
    char temp[64];  /* Enough for 64-bit in base 2 */
    size_t i = 0, len = 0;

    if (base < 2 || base > 36) {
        base = 10;
    }

    if (value == 0) {
        if (bufsize > 0) buf[0] = '0';
        return 1;
    }

    while (value > 0 && i < sizeof(temp)) {
        temp[i++] = digits[value % base];
        value /= base;
    }

    len = i;
    if (len > bufsize) len = bufsize;

    for (i = 0; i < len; i++) {
        buf[i] = temp[len - 1 - i];
    }

    return len;
}

static size_t add_grouping(char *buf, size_t len, char sep)
{
    size_t i, j, groups = (len - 1) / 3;
    size_t new_len = len + groups;

    if (groups == 0) return len;

    for (i = len, j = new_len; i > 0 && j > 0; ) {
        buf[--j] = buf[--i];
        if (i > 0 && (len - i) % 3 == 0) {
            buf[--j] = sep;
        }
    }

    return new_len;
}

static void format_integer(vsnprintf_state_t *state, uintmax_t value, 
                           int base, int flags, int width, int precision,
                           bool is_signed, bool is_negative)
{
    char num_buf[128];  /* Max for 128-bit with grouping */
    size_t num_len = 0, out_len = 0;
    size_t prefix_len = 0, pad_len = 0;
    char prefix[4] = {0};
    char pad_char = ' ';
    size_t i;

    if (is_signed) {
        if (is_negative) {
            prefix[prefix_len++] = '-';
        } else if (flags & FLAG_PLUS) {
            prefix[prefix_len++] = '+';
        } else if (flags & FLAG_SPACE) {
            prefix[prefix_len++] = ' ';
        }
    }

    if ((flags & FLAG_HASH) && value != 0) {
        if (base == 8) {
            prefix[prefix_len++] = '0';
        } else if (base == 16) {
            prefix[prefix_len++] = '0';
            prefix[prefix_len++] = (flags & FLAG_UPPERCASE) ? 'X' : 'x';
        } else if (base == 2) {
            prefix[prefix_len++] = '0';
            prefix[prefix_len++] = (flags & FLAG_UPPERCASE) ? 'B' : 'b';
        }
    }

    if (value == 0 && precision == 0 && (flags & FLAG_PRECISION)) {
        num_len = 0;
    } else {
        num_len = uint_to_string(num_buf, sizeof(num_buf), value, base, 
                                  flags & FLAG_UPPERCASE);
    }

    if (precision > 0 && (size_t)precision > num_len) {
        size_t prec_pad = (size_t)precision - num_len;
        if (prec_pad > sizeof(num_buf) - num_len) {
            prec_pad = sizeof(num_buf) - num_len;
        }
        memmove(num_buf + prec_pad, num_buf, num_len);
        memset(num_buf, '0', prec_pad);
        num_len += prec_pad;
    }

    if ((flags & FLAG_GROUP) && base == 10) {
        num_len = add_grouping(num_buf, num_len, ',');
    }

    out_len = prefix_len + num_len;

    if ((size_t)width > out_len) {
        pad_len = (size_t)width - out_len;
    }

    if ((flags & FLAG_ZERO) && !(flags & FLAG_PRECISION) && !(flags & FLAG_LEFT)) {
        pad_char = '0';
    }

    if (flags & FLAG_LEFT) {
        for (i = 0; i < prefix_len; i++) {
            output_char(state, prefix[i]);
        }
        output_string(state, num_buf, num_len);
        output_padding(state, ' ', pad_len);
    } else if (pad_char == '0') {
        for (i = 0; i < prefix_len; i++) {
            output_char(state, prefix[i]);
        }
        output_padding(state, '0', pad_len);
        output_string(state, num_buf, num_len);
    } else {
        output_padding(state, ' ', pad_len);
        for (i = 0; i < prefix_len; i++) {
            output_char(state, prefix[i]);
        }
        output_string(state, num_buf, num_len);
    }
}

static void format_float(vsnprintf_state_t *state, long double value,
                         char spec, int flags, int width, int precision)
{
    char buf[512];
    char *out = buf;
    size_t out_size = sizeof(buf);
    bool is_negative = false;
    bool is_inf = false, is_nan = false;
    long double abs_val;
    char sign_char = 0;
    size_t len = 0;
    size_t pad_len = 0;

    if (!(flags & FLAG_PRECISION)) {
        precision = 6;
    }

    if (precision < 0) precision = 0;
    if (precision > VSNPRINTF_MAX_PRECISION) precision = VSNPRINTF_MAX_PRECISION;

    if (isnan(value)) {
        is_nan = true;
        if (signbit(value)) is_negative = true;
    } else if (isinf(value)) {
        is_inf = true;
        is_negative = (value < 0);
    } else {
        is_negative = (value < 0);
    }

    if (is_negative) {
        sign_char = '-';
    } else if (flags & FLAG_PLUS) {
        sign_char = '+';
    } else if (flags & FLAG_SPACE) {
        sign_char = ' ';
    }

    if (is_nan) {
        const char *nan_str = (flags & FLAG_UPPERCASE) ? "NAN" : "nan";
        if (sign_char) {
            out[0] = sign_char;
            strncpy(out + 1, nan_str, out_size - 2);
            out[out_size - 1] = '\0';
            len = strlen(out);
        } else {
            strncpy(out, nan_str, out_size - 1);
            out[out_size - 1] = '\0';
            len = strlen(out);
        }
    } else if (is_inf) {
        const char *inf_str = (flags & FLAG_UPPERCASE) ? "INF" : "inf";
        if (sign_char) {
            out[0] = sign_char;
            strncpy(out + 1, inf_str, out_size - 2);
            out[out_size - 1] = '\0';
            len = strlen(out);
        } else {
            strncpy(out, inf_str, out_size - 1);
            out[out_size - 1] = '\0';
            len = strlen(out);
        }
    } else {
        abs_val = is_negative ? -value : value;

        switch (spec) {
            case 'f':
            case 'F': {
                long double int_part, frac_part;
                int i, digit;
                char *p = out;
                size_t remaining = out_size;

                if (sign_char) {
                    *p++ = sign_char;
                    remaining--;
                }

                frac_part = modfl(abs_val, &int_part);

                if (int_part == 0) {
                    if (remaining > 1) {
                        *p++ = '0';
                        remaining--;
                    }
                } else {
                    char int_buf[64];
                    int int_len = 0;
                    uintmax_t int_val = (uintmax_t)int_part;

                    while (int_val > 0 && int_len < 63) {
                        int_buf[int_len++] = '0' + (int_val % 10);
                        int_val /= 10;
                    }

                    while (int_len > 0 && remaining > 1) {
                        *p++ = int_buf[--int_len];
                        remaining--;
                    }
                }

                if (precision > 0 || (flags & FLAG_HASH)) {
                    if (remaining > 1) {
                        *p++ = '.';
                        remaining--;
                    }

                    for (i = 0; i < precision && remaining > 1; i++) {
                        frac_part *= 10;
                        digit = (int)frac_part;
                        *p++ = '0' + digit;
                        remaining--;
                        frac_part -= digit;
                    }

                    if (frac_part * 10 >= 5 && p > out && *(p-1) >= '0' && *(p-1) <= '9') {
                        char *r = p - 1;
                        while (r >= out && *r == '9') {
                            *r-- = '0';
                        }
                        if (r >= out && *r >= '0' && *r <= '9') {
                            (*r)++;
                        } else if (r < out && sign_char) {
                            memmove(out + 1, out, p - out);
                            out[0] = '1';
                            p++;
                        }
                    }
                }

                *p = '\0';
                len = p - out;
                break;
            }

            case 'e':
            case 'E': {
                char *p = out;
                size_t remaining = out_size;
                int exp_val = 0;
                long double mantissa = abs_val;
                int i, digit;

                if (sign_char) {
                    *p++ = sign_char;
                    remaining--;
                }

                if (mantissa != 0) {
                    while (mantissa >= 10) {
                        mantissa /= 10;
                        exp_val++;
                    }
                    while (mantissa < 1 && mantissa > 0) {
                        mantissa *= 10;
                        exp_val--;
                    }
                }

                digit = (int)mantissa;
                if (remaining > 1) {
                    *p++ = '0' + digit;
                    remaining--;
                }
                mantissa -= digit;

                if (precision > 0 || (flags & FLAG_HASH)) {
                    if (remaining > 1) {
                        *p++ = '.';
                        remaining--;
                    }

                    for (i = 0; i < precision && remaining > 1; i++) {
                        mantissa *= 10;
                        digit = (int)mantissa;
                        *p++ = '0' + digit;
                        remaining--;
                        mantissa -= digit;
                    }
                }

                if (remaining > 1) {
                    *p++ = (spec == 'E') ? 'E' : 'e';
                    remaining--;
                }
                if (remaining > 1) {
                    *p++ = (exp_val < 0) ? '-' : '+';
                    remaining--;
                }

                exp_val = exp_val < 0 ? -exp_val : exp_val;
                {
                    char exp_buf[8];
                    int exp_len = 0;
                    int e = exp_val;

                    if (e == 0) {
                        exp_buf[exp_len++] = '0';
                        exp_buf[exp_len++] = '0';
                    } else {
                        while (e > 0 && exp_len < 7) {
                            exp_buf[exp_len++] = '0' + (e % 10);
                            e /= 10;
                        }
                        while (exp_len < 2) {
                            exp_buf[exp_len++] = '0';
                        }
                    }

                    while (exp_len > 0 && remaining > 1) {
                        *p++ = exp_buf[--exp_len];
                        remaining--;
                    }
                }

                *p = '\0';
                len = p - out;
                break;
            }

            case 'g':
            case 'G': {
                long double abs_copy = abs_val;
                int exp_val = 0;

                if (abs_copy != 0) {
                    while (abs_copy >= 10) {
                        abs_copy /= 10;
                        exp_val++;
                    }
                    while (abs_copy < 1 && abs_copy > 0) {
                        abs_copy *= 10;
                        exp_val--;
                    }
                }

                if (exp_val < -4 || exp_val >= precision) {
                    int g_precision = precision > 0 ? precision - 1 : 0;
                    format_float(state, value, (spec == 'G') ? 'E' : 'e', 
                                flags, width, g_precision);
                    return;
                } else {
                    int f_precision = precision - (exp_val + 1);
                    if (f_precision < 0) f_precision = 0;
                    format_float(state, value, 'f', flags, width, f_precision);
                    return;
                }
            }

            case 'a':
            case 'A': {
                char *p = out;
                size_t remaining = out_size;
                int exp_val = 0;
                long double mantissa = abs_val;
                int i;
                const char *hex_digits = (spec == 'A') ? "0123456789ABCDEF" 
                                                        : "0123456789abcdef";

                if (sign_char) {
                    *p++ = sign_char;
                    remaining--;
                }

                if (remaining > 2) {
                    *p++ = '0';
                    *p++ = (spec == 'A') ? 'X' : 'x';
                    remaining -= 2;
                }

                if (mantissa == 0) {
                    if (remaining > 1) {
                        *p++ = '0';
                        remaining--;
                    }
                } else {
                    while (mantissa >= 2) {
                        mantissa /= 2;
                        exp_val++;
                    }
                    while (mantissa < 1 && mantissa > 0) {
                        mantissa *= 2;
                        exp_val--;
                    }

                    if (remaining > 1) {
                        *p++ = '1';
                        remaining--;
                    }

                    mantissa -= 1;  /* Remove the leading 1 */

                    if (precision > 0 || (flags & FLAG_HASH)) {
                        if (remaining > 1) {
                            *p++ = '.';
                            remaining--;
                        }

                        for (i = 0; i < precision && remaining > 1; i++) {
                            mantissa *= 16;
                            int digit = (int)mantissa;
                            *p++ = hex_digits[digit];
                            remaining--;
                            mantissa -= digit;
                        }
                    }
                }

                if (remaining > 1) {
                    *p++ = (spec == 'A') ? 'P' : 'p';
                    remaining--;
                }
                if (remaining > 1) {
                    *p++ = (exp_val < 0) ? '-' : '+';
                    remaining--;
                }

                exp_val = exp_val < 0 ? -exp_val : exp_val;
                {
                    char exp_buf[8];
                    int exp_len = 0;
                    int e = exp_val;

                    do {
                        exp_buf[exp_len++] = '0' + (e % 10);
                        e /= 10;
                    } while (e > 0 && exp_len < 7);

                    while (exp_len > 0 && remaining > 1) {
                        *p++ = exp_buf[--exp_len];
                        remaining--;
                    }
                }

                *p = '\0';
                len = p - out;
                break;
            }

            default:
                strncpy(out, "0", out_size - 1);
                out[out_size - 1] = '\0';
                len = 1;
                break;
        }
    }

    if ((size_t)width > len) {
        pad_len = (size_t)width - len;
    }

    if (flags & FLAG_LEFT) {
        output_string(state, out, len);
        output_padding(state, ' ', pad_len);
    } else {
        if ((flags & FLAG_ZERO) && !is_nan && !is_inf) {
            if (sign_char) {
                output_char(state, sign_char);
                out++;  /* Skip sign in string */
                len--;
            }
            output_padding(state, '0', pad_len);
            output_string(state, out, len);
        } else {
            output_padding(state, ' ', pad_len);
            output_string(state, out, len);
        }
    }
}

static void format_string(vsnprintf_state_t *state, const char *str,
                          int flags, int width, int precision)
{
    size_t str_len;
    size_t pad_len = 0;

    if (!str) {
        str = "(null)";
    }

    str_len = strlen(str);

    if ((flags & FLAG_PRECISION) && (size_t)precision < str_len) {
        str_len = (size_t)precision;
    }

    if ((size_t)width > str_len) {
        pad_len = (size_t)width - str_len;
    }

    if (flags & FLAG_LEFT) {
        output_string(state, str, str_len);
        output_padding(state, ' ', pad_len);
    } else {
        output_padding(state, ' ', pad_len);
        output_string(state, str, str_len);
    }
}

static void format_pointer(vsnprintf_state_t *state, void *ptr,
                           int flags, int width)
{
    if (!ptr) {
        format_string(state, "(nil)", flags, width, -1);
        return;
    }

    format_integer(state, (uintptr_t)ptr, 16, 
                   flags | FLAG_HASH, width, 
                   (int)(sizeof(void*) * 2), false, false);
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
    vsnprintf_state_t state = {0};
    size_t i = 0;
    int result;

    state.buffer = str;
    state.size = size;
    state.pos = 0;
    state.total = 0;
    state.overflow = false;
    state.error = false;

    if (!format) {
        format = "";
    }

    while (format[i] != '\0') {
        if (format[i] != '%') {
            output_char(&state, format[i]);
            i++;
            continue;
        }

        i++;  /* Skip '%' */

        if (format[i] == '\0') {
            output_char(&state, '%');
            break;
        }

        if (format[i] == '%') {
            output_char(&state, '%');
            i++;
            continue;
        }

        int flags = 0;
        bool parsing_flags = true;
        while (parsing_flags) {
            switch (format[i]) {
                case '-': flags |= FLAG_LEFT; i++; break;
                case '+': flags |= FLAG_PLUS; i++; break;
                case ' ': flags |= FLAG_SPACE; i++; break;
                case '0': flags |= FLAG_ZERO; i++; break;
                case '#': flags |= FLAG_HASH; i++; break;
                case '\'': flags |= FLAG_GROUP; i++; break;
                default: parsing_flags = false; break;
            }
        }

        int width = 0;
        if (format[i] == '*') {
            width = va_arg(ap, int);
            if (width < 0) {
                width = -width;
                flags |= FLAG_LEFT;
            }
            i++;
        } else {
            width = parse_format_int(format, &i, 0);
        }

        if (width > VSNPRINTF_MAX_WIDTH) {
            width = VSNPRINTF_MAX_WIDTH;
        }

        int precision = 0;
        if (format[i] == '.') {
            i++;
            flags |= FLAG_PRECISION;
            if (format[i] == '*') {
                precision = va_arg(ap, int);
                if (precision < 0) {
                    precision = 0;
                    flags &= ~FLAG_PRECISION;
                }
                i++;
            } else {
                precision = parse_format_int(format, &i, 0);
            }

            if (precision > VSNPRINTF_MAX_PRECISION) {
                precision = VSNPRINTF_MAX_PRECISION;
            }
        }

        length_modifier_t length = LENGTH_NONE;
        switch (format[i]) {
            case 'h':
                i++;
                if (format[i] == 'h') {
                    length = LENGTH_HH;
                    i++;
                } else {
                    length = LENGTH_H;
                }
                break;
            case 'l':
                i++;
                if (format[i] == 'l') {
                    length = LENGTH_LL;
                    i++;
                } else {
                    length = LENGTH_L;
                }
                break;
            case 'j':
                length = LENGTH_J;
                i++;
                break;
            case 'z':
                length = LENGTH_Z;
                i++;
                break;
            case 't':
                length = LENGTH_T;
                i++;
                break;
            case 'L':
                length = LENGTH_LDBL;
                i++;
                break;
        }

        char spec = format[i];
        if (spec == '\0') {
            break;
        }
        i++;

        if (spec >= 'A' && spec <= 'Z') {
            flags |= FLAG_UPPERCASE;
        }

        switch (spec) {
            case 'd':
            case 'i': {
                intmax_t value = 0;
                bool is_negative = false;

                switch (length) {
                    case LENGTH_HH: 
                        value = (signed char)va_arg(ap, int); 
                        break;
                    case LENGTH_H: 
                        value = (short)va_arg(ap, int); 
                        break;
                    case LENGTH_L: 
                        value = va_arg(ap, long); 
                        break;
                    case LENGTH_LL: 
                        value = va_arg(ap, long long); 
                        break;
                    case LENGTH_J: 
                        value = va_arg(ap, intmax_t); 
                        break;
                    case LENGTH_Z: 
                        value = (intmax_t)va_arg(ap, size_t); 
                        break;
                    case LENGTH_T: 
                        value = va_arg(ap, ptrdiff_t); 
                        break;
                    default: 
                        value = va_arg(ap, int); 
                        break;
                }

                is_negative = (value < 0);
                format_integer(&state, is_negative ? -(uintmax_t)value : (uintmax_t)value,
                              10, flags | FLAG_SIGNED, width, precision, true, is_negative);
                break;
            }

            case 'u':
            case 'o':
            case 'x':
            case 'X': {
                uintmax_t value = 0;
                int base = (spec == 'u') ? 10 : (spec == 'o') ? 8 : 16;

                switch (length) {
                    case LENGTH_HH: 
                        value = (unsigned char)va_arg(ap, unsigned int); 
                        break;
                    case LENGTH_H: 
                        value = (unsigned short)va_arg(ap, unsigned int); 
                        break;
                    case LENGTH_L: 
                        value = va_arg(ap, unsigned long); 
                        break;
                    case LENGTH_LL: 
                        value = va_arg(ap, unsigned long long); 
                        break;
                    case LENGTH_J: 
                        value = va_arg(ap, uintmax_t); 
                        break;
                    case LENGTH_Z: 
                        value = va_arg(ap, size_t); 
                        break;
                    case LENGTH_T: 
                        value = (uintmax_t)va_arg(ap, ptrdiff_t); 
                        break;
                    default: 
                        value = va_arg(ap, unsigned int); 
                        break;
                }

                format_integer(&state, value, base, flags, width, precision, false, false);
                break;
            }

            case 'f':
            case 'F':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
            case 'a':
            case 'A': {
                long double value = 0;

                switch (length) {
                    case LENGTH_LDBL:
                        value = va_arg(ap, long double);
                        break;
                    default:
                        value = va_arg(ap, double);
                        break;
                }

                format_float(&state, value, spec, flags, width, precision);
                break;
            }

            case 'c': {
                char c = (char)va_arg(ap, int);
                if (flags & FLAG_LEFT) {
                    output_char(&state, c);
                    output_padding(&state, ' ', width > 1 ? (size_t)width - 1 : 0);
                } else {
                    output_padding(&state, ' ', width > 1 ? (size_t)width - 1 : 0);
                    output_char(&state, c);
                }
                break;
            }

            case 's': {
                const char *str_val = va_arg(ap, const char *);
                format_string(&state, str_val, flags, width, precision);
                break;
            }

            case 'p': {
                void *ptr = va_arg(ap, void *);
                format_pointer(&state, ptr, flags, width);
                break;
            }

            case 'n': {
                int *count_ptr = va_arg(ap, int *);
                if (count_ptr) {
                    *count_ptr = (int)state.total;
                }
                break;
            }

            case 'm': {
                const char *err_str = strerror(errno);
                format_string(&state, err_str, flags, width, precision);
                break;
            }

            default: {
                output_char(&state, '%');
                if (format[i-1] != '%') {
                    output_char(&state, format[i-1]);
                }
                break;
            }
        }
    }

    if (state.buffer && state.size > 0) {
        if (state.pos < state.size) {
            state.buffer[state.pos] = '\0';
        } else {
            state.buffer[state.size - 1] = '\0';
        }
    }

    result = (state.total > INT_MAX) ? INT_MAX : (int)state.total;

    return result;
}
