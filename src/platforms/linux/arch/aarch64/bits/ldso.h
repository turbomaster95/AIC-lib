#ifndef AAL_LDSO_H
#define AAL_LDSO_H

#include <stdint.h>

#define AAL_DEFINE_LDSO_ENTRY() \
    __asm__( \
        ".text\n" \
        ".global _start\n" \
        ".type _start, %function\n" \
        "_start:\n" \
        "    mov x0, sp\n" \
        "    bl _start_c\n" \
        "1:  b 1b\n" \
    )


#define AAL_DEFINE_DL_RUNTIME_RESOLVE() \
    __asm__( \
        ".text\n" \
        ".global _dl_runtime_resolve\n" \
        ".type _dl_runtime_resolve, %function\n" \
        "_dl_runtime_resolve:\n" \
        "    sub sp, sp, #80\n" \
        "    stp x0, x1, [sp, #0]\n" \
        "    stp x2, x3, [sp, #16]\n" \
        "    stp x4, x5, [sp, #32]\n" \
        "    stp x6, x7, [sp, #48]\n" \
        "    stp x8, x30, [sp, #64]\n" \
        "\n" \
        "    mov x0, x16\n" \
        "    mov x1, x17\n" \
        "    bl _dl_fixup\n" \
        "\n" \
        "    mov x16, x0\n" \
        "\n" \
        "    ldp x8, x30, [sp, #64]\n" \
        "    ldp x6, x7, [sp, #48]\n" \
        "    ldp x4, x5, [sp, #32]\n" \
        "    ldp x2, x3, [sp, #16]\n" \
        "    ldp x0, x1, [sp, #0]\n" \
        "    add sp, sp, #80\n" \
        "\n" \
        "    br x16\n" \
    )

static inline __attribute__((noreturn)) void aal_ldso_jump_to_entry(uintptr_t *sp, uintptr_t entry) {
    __asm__ __volatile__(
        "mov sp, %[sp]\n\t"
        "mov x16, %[entry]\n\t"
        "mov x0, xzr\n\t"
        "mov x1, xzr\n\t"
        "mov x2, xzr\n\t"
        "mov x3, xzr\n\t"
        "mov x4, xzr\n\t"
        "mov x5, xzr\n\t"
        "mov x6, xzr\n\t"
        "mov x7, xzr\n\t"
        "mov x8, xzr\n\t"
        "mov x9, xzr\n\t"
        "mov x10, xzr\n\t"
        "mov x11, xzr\n\t"
        "mov x12, xzr\n\t"
        "mov x13, xzr\n\t"
        "mov x14, xzr\n\t"
        "mov x15, xzr\n\t"
        "mov x17, xzr\n\t"
        "mov x29, xzr\n\t"
        "mov x30, xzr\n\t"
        "br x16\n\t"
        :
        : [sp] "r"(sp), [entry] "r"(entry)
        : "x16", "memory"
    );
    __builtin_unreachable();
}

#endif /* AAL_LDSO_H */

