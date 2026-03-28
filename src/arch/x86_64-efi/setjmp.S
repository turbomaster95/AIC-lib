/*
 * x86_64-efi setjmp/longjmp implementation
 */

.section .text

/*
 * int setjmp(jmp_buf env)
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
    
    /* Save return address */
    movq (%rsp), %rax
    movq %rax, 56(%rdi)
    
    /* Save flags register */
    pushfq
    popq %rax
    movq %rax, 64(%rdi)
    
    /* Return 0 */
    xorl %eax, %eax
    ret

/*
 * void longjmp(jmp_buf env, int val)
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
    
    /* Set return value */
    movl %esi, %eax
    testl %eax, %eax
    jnz 1f
    movl $1, %eax
1:
    /* Jump back by pushing saved RIP and returning */
    movq 56(%rdi), %rcx
    pushq %rcx
    ret
