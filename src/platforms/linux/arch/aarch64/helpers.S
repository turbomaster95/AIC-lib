.section .text
.global __aarch64_ldset4_acq_rel
.type __aarch64_ldset4_acq_rel, %function

// uint32_t __aarch64_ldset4_acq_rel(uint32_t val, uint32_t *ptr)
// Performs atomic fetch-and-OR (*ptr |= val) with acquire-release ordering.
// Inputs:  w0 = value to OR into target
//          x1 = ptr to 32-bit integer
// Output:  w0 = original value at *ptr before the operation
__aarch64_ldset4_acq_rel:
.L_ldset_loop:
    ldaxr   w2, [x1]
    orr     w3, w2, w0
    stlxr   w4, w3, [x1]
    cbnz    w4, .L_ldset_loop
    mov     w0, w2
    ret

.size __aarch64_ldset4_acq_rel, .-__aarch64_ldset4_acq_rel


.global __aarch64_cas4_acq_rel
.type __aarch64_cas4_acq_rel, %function

// uint32_t __aarch64_cas4_acq_rel(uint32_t expected, uint32_t desired, uint32_t *ptr)
// Performs atomic Compare-and-Swap with acquire-release ordering.
// Inputs:  w0 = expected value
//          w1 = desired value
//          x2 = ptr to 32-bit integer
// Output:  w0 = value at *ptr before the CAS attempt.
//               If w0 == expected, the swap succeeded.
__aarch64_cas4_acq_rel:
.L_cas_loop:
    ldaxr   w3, [x2]
    cmp     w3, w0
    b.ne    .L_cas_exit
    stlxr   w4, w1, [x2]
    cbnz    w4, .L_cas_loop
.L_cas_exit:
    mov     w0, w3
    ret

.size __aarch64_cas4_acq_rel, .-__aarch64_cas4_acq_rel

