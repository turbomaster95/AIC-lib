/*
 * AArch64 setjmp/longjmp implementation
 * 
 * jmp_buf layout (168 bytes):
 *   [0-79]   x19-x28 (10 registers * 8 bytes)
 *   [80-87]  x29 (frame pointer)
 *   [88-95]  x30 (link register)
 *   [96-103] sp (stack pointer)
 *   [104-111] pc (program counter - stored in x0 on return)
 *   [112-175] d8-d15 (8 FP registers * 8 bytes)
 */

.section .text

/*
 * int setjmp(jmp_buf env)
 * 
 * Saves the current execution context to env.
 * Returns 0 when called directly, non-zero when returning from longjmp.
 */
.globl setjmp
.type setjmp, %function
setjmp:
    /* Save callee-saved registers x19-x28 */
    str x19, [x0, #0]
    str x20, [x0, #8]
    str x21, [x0, #16]
    str x22, [x0, #24]
    str x23, [x0, #32]
    str x24, [x0, #40]
    str x25, [x0, #48]
    str x26, [x0, #56]
    str x27, [x0, #64]
    str x28, [x0, #72]
    
    /* Save frame pointer and link register */
    str x29, [x0, #80]
    str x30, [x0, #88]
    
    /* Save stack pointer */
    mov x1, sp
    str x1, [x0, #96]
    
    /* Save return address (pc) - it's in x30 (LR) at function entry */
    /* We already saved it above, but we need to store the caller's PC */
    /* The PC we want is actually the LR at entry, which we saved */
    
    /* Save FP registers d8-d15 */
    str d8, [x0, #112]
    str d9, [x0, #120]
    str d10, [x0, #128]
    str d11, [x0, #136]
    str d12, [x0, #144]
    str d13, [x0, #152]
    str d14, [x0, #160]
    str d15, [x0, #168]
    
    /* Return 0 to indicate direct call */
    mov x0, #0
    ret

/*
 * void longjmp(jmp_buf env, int val)
 * 
 * Restores the execution context from env.
 * The setjmp() call will appear to return with value val (or 1 if val is 0).
 * 
 * Arguments on entry:
 *   x0 = env (jmp_buf)
 *   x1 = val (return value)
 */
.globl longjmp
.type longjmp, %function
longjmp:
    /* Save the return value before we overwrite x1 */
    mov x2, x1
    
    /* Restore callee-saved registers x19-x28 */
    ldr x19, [x0, #0]
    ldr x20, [x0, #8]
    ldr x21, [x0, #16]
    ldr x22, [x0, #24]
    ldr x23, [x0, #32]
    ldr x24, [x0, #40]
    ldr x25, [x0, #48]
    ldr x26, [x0, #56]
    ldr x27, [x0, #64]
    ldr x28, [x0, #72]
    
    /* Restore frame pointer and link register */
    ldr x29, [x0, #80]
    ldr x30, [x0, #88]
    
    /* Restore stack pointer - must use intermediate register */
    ldr x1, [x0, #96]
    mov sp, x1
    
    /* Restore FP registers d8-d15 */
    ldr d8, [x0, #112]
    ldr d9, [x0, #120]
    ldr d10, [x0, #128]
    ldr d11, [x0, #136]
    ldr d12, [x0, #144]
    ldr d13, [x0, #152]
    ldr d14, [x0, #160]
    ldr d15, [x0, #168]
    
    /* Set return value - must be non-zero (use saved value in x2) */
    mov x0, x2
    cbnz x2, 1f
    mov x0, #1
1:
    /* Return to the saved location by branching to saved LR */
    br x30
