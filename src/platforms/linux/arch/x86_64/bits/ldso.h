#ifndef AAL_LDSO_H
#define AAL_LDSO_H

#include <stdint.h>

#define AAL_DEFINE_LDSO_ENTRY()                         \
    __asm__(                                             \
        ".text\n"                                       \
        ".global _start\n"                              \
        ".type _start,@function\n"                      \
        "_start:\n"                                     \
        "    mov %rsp,%rdi\n"                           \
        "    and $-16,%rsp\n"                           \
        "    call _start_c\n"                           \
        "    hlt\n"                                     \
    )

#define AAL_DEFINE_DL_RUNTIME_RESOLVE()                 \
    __asm__(                                             \
        ".text\n"                                       \
        ".global _dl_runtime_resolve\n"                 \
        ".type _dl_runtime_resolve,@function\n"         \
        "_dl_runtime_resolve:\n"                        \
        "    subq $64,%rsp\n"                           \
        "    movq %rax,0(%rsp)\n"                       \
        "    movq %rcx,8(%rsp)\n"                       \
        "    movq %rdx,16(%rsp)\n"                      \
        "    movq %rsi,24(%rsp)\n"                      \
        "    movq %rdi,32(%rsp)\n"                      \
        "    movq %r8,40(%rsp)\n"                       \
        "    movq %r9,48(%rsp)\n"                       \
        "    movq %r11,56(%rsp)\n"                      \
        "    movq 80(%rsp),%rdi\n"                      \
        "    movq 72(%rsp),%rsi\n"                      \
        "    call _dl_fixup\n"                          \
        "    movq %rax,%r11\n"                          \
        "    movq 0(%rsp),%rax\n"                       \
        "    movq 8(%rsp),%rcx\n"                       \
        "    movq 16(%rsp),%rdx\n"                      \
        "    movq 24(%rsp),%rsi\n"                      \
        "    movq 32(%rsp),%rdi\n"                      \
        "    movq 40(%rsp),%r8\n"                       \
        "    movq 48(%rsp),%r9\n"                       \
        "    movq 56(%rsp),%r11\n"                      \
        "    addq $64,%rsp\n"                           \
        "    addq $16,%rsp\n"                           \
        "    jmp *%r11\n"                               \
    )

static inline __attribute__((noreturn))
void aal_ldso_jump_to_entry_with_fini(uintptr_t *sp, uintptr_t entry,
                                      uintptr_t rtld_fini)
{
    __asm__ __volatile__(
        "mov %[sp], %%rsp\n\t"
        "xor %%rbp, %%rbp\n\t"
        "xor %%rax, %%rax\n\t"
        "xor %%rbx, %%rbx\n\t"
        "xor %%rcx, %%rcx\n\t"
        "xor %%rsi, %%rsi\n\t"
        "xor %%rdi, %%rdi\n\t"
        "xor %%r8, %%r8\n\t"
        "xor %%r9, %%r9\n\t"
        "xor %%r10, %%r10\n\t"
        "xor %%r11, %%r11\n\t"
        "xor %%r12, %%r12\n\t"
        "xor %%r13, %%r13\n\t"
        "xor %%r14, %%r14\n\t"
        "xor %%r15, %%r15\n\t"
        "mov %[fini], %%rdx\n\t"
        "jmp *%[entry]\n\t"
        :
        : [sp] "r"(sp), [entry] "r"(entry), [fini] "r"(rtld_fini)
        : "memory"
    );
    __builtin_unreachable();
}

static inline __attribute__((noreturn))
void aal_ldso_jump_to_entry(uintptr_t *sp, uintptr_t entry)
{
    aal_ldso_jump_to_entry_with_fini(sp, entry, 0);
}

#endif
