#ifndef AAL_LDSO_H
#define AAL_LDSO_H

#include <stdint.h>

#define AAL_DEFINE_LDSO_ENTRY() \
    __asm__( \
        ".text\n" \
        ".global _start\n" \
        ".type _start, @function\n" \
        "_start:\n" \
        "    mov %rsp, %rdi\n" \
        "    and $-16, %rsp\n" \
        "    call _start_c\n" \
        "    hlt\n" \
    )

#define AAL_DEFINE_DL_RUNTIME_RESOLVE() \
    __asm__( \
        ".global _dl_runtime_resolve\n" \
        ".type _dl_runtime_resolve, @function\n" \
        "_dl_runtime_resolve:\n" \
        "    pushq %rbx\n" \
        "    movq 8(%rsp), %rbx\n" \
        "    pushq %rbp\n" \
        "    pushq %r12\n" \
        "    pushq %r13\n" \
        "    pushq %r14\n" \
        "    pushq %r15\n" \
        "    movq 16(%rsp), %rdi\n" \
        "    movq 24(%rsp), %rsi\n" \
        "    call _dl_fixup\n" \
        "    movq %rax, %r11\n" \
        "    popq %r15\n" \
        "    popq %r14\n" \
        "    popq %r13\n" \
        "    popq %r12\n" \
        "    popq %rbp\n" \
        "    popq %rbx\n" \
        "    addq $16, %rsp\n" \
        "    jmp *%r11\n" \
    )

static inline __attribute__((noreturn)) void aal_ldso_jump_to_entry(uintptr_t *sp, uintptr_t entry) {
    __asm__ __volatile__(
        "mov %[sp], %%rsp\n\t"
        "mov %[entry], %%r12\n\t"
        "xor %%rax, %%rax\n\t"
        "xor %%rbx, %%rbx\n\t"
        "xor %%rcx, %%rcx\n\t"
        "xor %%rdx, %%rdx\n\t"
        "xor %%rsi, %%rsi\n\t"
        "xor %%rdi, %%rdi\n\t"
        "xor %%rbp, %%rbp\n\t"
        "jmp *%%r12\n\t"
        :
        : [sp] "r"(sp), [entry] "r"(entry)
        : "r12", "memory"
    );
    __builtin_unreachable();
}

#endif /* AAL_LDSO_H */

