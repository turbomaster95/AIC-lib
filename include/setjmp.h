#ifndef _SETJMP_H
#define _SETJMP_H

#include <bits/types.h>

/* 
 * jmp_buf - Storage for saved register state
 * Layout is architecture-specific.
 * 
 * For AArch64, we need to save:
 * - x19-x28 (10 callee-saved registers) = 80 bytes
 * - x29 (frame pointer) = 8 bytes
 * - x30 (link register) = 8 bytes
 * - sp (stack pointer) = 8 bytes
 * - d8-d15 (8 FP registers, optional) = 64 bytes
 * Total: ~168 bytes, rounded to 256 for alignment
 *
 * For x86_64, we need to save:
 * - rbx, rbp, r12-r15 (7 registers) = 56 bytes
 * - rsp (stack pointer) = 8 bytes
 * - rip (return address) = 8 bytes
 * Total: ~72 bytes, rounded to 128 for alignment
 */

#if defined(__aarch64__)
typedef struct {
    unsigned long x[19];  /* x19-x28, x29(FP), x30(LR), sp, pc */
    unsigned long d[8];   /* d8-d15 (FP registers) */
} jmp_buf[1];
#elif defined(__x86_64__)
typedef struct {
    unsigned long r[14];  /* rbx, rbp, r12-r15, rsp, rip, rflags, and padding */
} jmp_buf[1];
#else
/* Default fallback */
typedef struct {
    unsigned long data[32];
} jmp_buf[1];
#endif

/* Function declarations */
int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

#endif /* _SETJMP_H */
