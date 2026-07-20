/* aic-ld-start.s - Entry point for aic-ld.so */
    .text
    .globl _start
    .type _start, @function

_start:
    /* Stack on entry (from kernel or execve):
     *   rsp -> argc
     *   rsp+8 -> argv[0]
     *   ...
     *   argv[argc] = NULL
     *   envp[0]
     *   ...
     *   envp[envc] = NULL
     *   auxv[0] (AT_NULL terminated)
     *
     * We need to pass: sp, &_DYNAMIC to _dl_start_c
     * Per AMD64 SysV ABI: %rdi = arg1, %rsi = arg2
     */

    /* Save stack pointer in %rdi (first arg) */
    mov     %rsp, %rdi

    /* Load address of _DYNAMIC into %rsi (second arg).
     * _DYNAMIC is a linker-defined symbol at the start of
     * our .dynamic section. We use RIP-relative addressing. */
    leaq    _DYNAMIC(%rip), %rsi

    /* Align stack to 16 bytes before call */
    andq    $-16, %rsp

    /* Call the C entry point */
    call    _dl_start_c

    /* _dl_start_c should not return. If it does, exit cleanly.
     * SYS_exit = 60 */
    movq    $60, %rax
    movq    $127, %rdi
    syscall

    .size _start, .-_start
