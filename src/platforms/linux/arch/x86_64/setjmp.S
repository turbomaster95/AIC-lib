.section .text

/*
 * int _setjmp(jmp_buf env)
 * 
 * Saves the current execution context to env.
 * Returns 0 when called directly, non-zero when returning from _longjmp.
 */
.globl _setjmp
.type _setjmp, @function
_setjmp:
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
 * void _longjmp(jmp_buf env, int val)
 * 
 * Restores the execution context from env.
 * The _setjmp() call will appear to return with value val (or 1 if val is 0).
 */
.globl _longjmp
.type _longjmp, @function
_longjmp:
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
