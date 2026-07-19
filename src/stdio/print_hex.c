#include <internal/pal.h>
#include <stdio.h>

void print_hex(unsigned long val) {
    char buf[19]; // "0x" + 16 hex digits + null
    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 17; i >= 2; i--) {
        int nibble = val & 0xF;
        buf[i] = (nibble < 10) ? (nibble + '0') : (nibble - 10 + 'a');
        val >>= 4;
    }
    buf[18] = '\0';

    pal_write(1, (void*)buf, sizeof((long)buf));
}
