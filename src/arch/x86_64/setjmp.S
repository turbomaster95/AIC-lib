/*
 * x86_64 setjmp/longjmp implementation
 * 
 * jmp_buf layout (112 bytes):
 *   [0-7]    rbx
 *   [8-15]   rbp
 *   [16-23]  r12
 *   [24-31]  r13
 *   [32-39]  r14
 *   [40-47]  r15
 *   [48-55]  rsp (stack pointer)
 *   [56-63]  rip (return address)
 *   [64-71]  rflags
 *   [72-111] padding and optional FP registers
 */

.section .text

/*
 * int setjmp(jmp_buf env)
 * 
 * Saves the current execution context to env.
 * Returns 0 when called directly, non-zero when returning from longjmp.
 */
.globl setjmp
.type setjmp, @function
setjmp:
    /* Save callee-saved registers */
    movq %rbx, 0(%rdi)
    movq %rbp, 8(%rdi)
    movq %r12, 16(%rdi)
    movq %r13, 24(%rdi)
    movq %r14, 32(%rdi)
    movq %r15, 40(%rdi)
    
    /* Save stack pointer */
    movq %rsp, 48(%rdi)
    
    /* Save return address (from stack) */
    movq (%rsp), %rax
    movq %rax, 56(%rdi)
    
    /* Save flags register */
    pushfq
    popq %rax
    movq %rax, 64(%rdi)
    
    /* Return 0 to indicate direct call */
    xorl %eax, %eax
    ret

/*
 * void longjmp(jmp_buf env, int val)
 * 
 * Restores the execution context from env.
 * The setjmp() call will appear to return with value val (or 1 if val is 0).
 */
.globl longjmp
.type longjmp, @function
longjmp:
    /* Restore callee-saved registers */
    movq 0(%rdi), %rbx
    movq 8(%rdi), %rbp
    movq 16(%rdi), %r12
    movq 24(%rdi), %r13
    movq 32(%rdi), %r14
    movq 40(%rdi), %r15
    
    /* Restore stack pointer */
    movq 48(%rdi), %rsp
    
    /* Restore flags register */
    movq 64(%rdi), %rax
    pushq %rax
    popfq
    
    /* Set return value - must be non-zero */
    movl %esi, %eax
    testl %eax, %eax
    jnz 1f
    movl $1, %eax
1:
    /* Return to saved location by pushing saved RIP and returning */
    movq 56(%rdi), %rcx
    pushq %rcx
    ret
