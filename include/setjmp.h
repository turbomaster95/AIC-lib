#ifndef _SETJMP_H
#define _SETJMP_H

#include <bits/types.h>

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
int _setjmp(jmp_buf env);
void _longjmp(jmp_buf env, int val);

#define setjmp(env) _setjmp(env);
#define longjmp(env, val) _longjmp(env, val)

#endif /* _SETJMP_H */
