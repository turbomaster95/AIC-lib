.global fma
.type fma, @function
fma:
    fmadd d0, d0, d1, d2
    ret
.size fma, .-fma

.global fmaf
.type fmaf, @function
fmaf:
    fmadd s0, s0, s1, s2
    ret
.size fmaf, .-fmaf
