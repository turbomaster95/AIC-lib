#include <internal/syscall.h> // This is the missing link
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

    // Use the generic syscall we just built
    __syscall_gen(SYS_write, 1, (long)buf, 18, 0, 0, 0);
}
