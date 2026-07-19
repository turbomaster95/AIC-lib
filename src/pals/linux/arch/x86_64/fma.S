#if defined(__FMA__)
.global fma
.type fma, @function
fma:
    vfmadd213pd %xmm0, %xmm1, %xmm2
    vmovapd %xmm2, %xmm0
    ret
.size fma, .-fma

.global fmaf
.type fmaf, @function
fmaf:
    vfmadd213ps %xmm0, %xmm1, %xmm2
    vmovaps %xmm2, %xmm0
    ret
.size fmaf, .-fmaf
#endif
