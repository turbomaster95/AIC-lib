.global sqrt
.type sqrt, @function
sqrt:
    sqrtsd %xmm0, %xmm0
    ret
.size sqrt, .-sqrt

.global sqrtf
.type sqrtf, @function
sqrtf:
    sqrtss %xmm0, %xmm0
    ret
.size sqrtf, .-sqrtf
