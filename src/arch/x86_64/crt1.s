.section .text
.global _start
_start:
    xor %rbp, %rbp
    mov (%rsp), %rdi
    lea 8(%rsp), %rsi
    call main
    mov %rax, %rdi
    mov $60, %rax
    syscall