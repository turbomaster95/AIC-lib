#include <stdio.h>
#include <stdarg.h>
#include <string.h>

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
    size_t i = 0;
    const char *p = format;
    char buf[64];

    while (*p && i < size - 1) {
        if (*p == '%') {
            p++;
            int is_long = 0;
            if (*p == 'l') {
                is_long = 1;
                p++;
            }

            switch (*p) {
                case 'd': {
                    long val = is_long ? va_arg(ap, long) : (long)va_arg(ap, int);
                    if (val < 0) {
                        if (i < size - 1) str[i++] = '-';
                        val = -val;
                    }
                    unsigned long uval = (unsigned long)val;
                    char *digits = "0123456789";
                    char *dp = buf + sizeof(buf) - 1;
                    *dp = '\0';
                    if (uval == 0) {
                        *--dp = '0';
                    } else {
                        while (uval > 0) {
                            *--dp = digits[uval % 10];
                            uval /= 10;
                        }
                    }
                    for (size_t j = 0; dp[j] && i < size - 1; j++) {
                        str[i++] = dp[j];
                    }
                    break;
                }
                case 'u': {
                    unsigned long val = is_long ? va_arg(ap, unsigned long) : (unsigned long)va_arg(ap, unsigned int);
                    char *digits = "0123456789";
                    char *dp = buf + sizeof(buf) - 1;
                    *dp = '\0';
                    if (val == 0) {
                        *--dp = '0';
                    } else {
                        while (val > 0) {
                            *--dp = digits[val % 10];
                            val /= 10;
                        }
                    }
                    for (size_t j = 0; dp[j] && i < size - 1; j++) {
                        str[i++] = dp[j];
                    }
                    break;
                }
                case 'x': {
                    unsigned long val = is_long ? va_arg(ap, unsigned long) : (unsigned long)va_arg(ap, unsigned int);
                    char *digits = "0123456789abcdef";
                    char *dp = buf + sizeof(buf) - 1;
                    *dp = '\0';
                    if (val == 0) {
                        *--dp = '0';
                    } else {
                        while (val > 0) {
                            *--dp = digits[val % 16];
                            val /= 16;
                        }
                    }
                    for (size_t j = 0; dp[j] && i < size - 1; j++) {
                        str[i++] = dp[j];
                    }
                    break;
                }
                case 'p': {
                    void *val = va_arg(ap, void *);
                    if (val == 0) {
                        const char *nil = "(nil)";
                        for (size_t j = 0; nil[j] && i < size - 1; j++) {
                            str[i++] = nil[j];
                        }
                    } else {
                        unsigned long addr = (unsigned long)val;
                        char *digits = "0123456789abcdef";
                        char *dp = buf + sizeof(buf) - 1;
                        *dp = '\0';
                        if (addr == 0) {
                            *--dp = '0';
                        } else {
                            while (addr > 0) {
                                *--dp = digits[addr % 16];
                                addr /= 16;
                            }
                        }
                        for (size_t j = 0; dp[j] && i < size - 1; j++) {
                            str[i++] = dp[j];
                        }
                    }
                    break;
                }
                case 's': {
                    const char *s = va_arg(ap, const char *);
                    if (s) {
                        for (size_t j = 0; s[j] && i < size - 1; j++) {
                            str[i++] = s[j];
                        }
                    } else {
                        const char *nil = "(null)";
                        for (size_t j = 0; nil[j] && i < size - 1; j++) {
                            str[i++] = nil[j];
                        }
                    }
                    break;
                }
                case 'c': {
                    str[i++] = (char)va_arg(ap, int);
                    break;
                }
                case '%': {
                    str[i++] = '%';
                    break;
                }
                default: {
                    /* Unknown format specifier, copy literally */
                    str[i++] = '%';
                    if (is_long && i < size - 1) str[i++] = 'l';
                    if (i < size - 1) str[i++] = *p;
                    break;
                }
            }
        } else {
            str[i++] = *p;
        }
        p++;
    }
    str[i] = '\0';
    return (int)i;
}
