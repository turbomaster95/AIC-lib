.section .text
.global _start
_start:
    mov x29, #0
    ldr x0, [sp]
    add x1, sp, #8
    bl main
    mov x8, #93
    svc 0