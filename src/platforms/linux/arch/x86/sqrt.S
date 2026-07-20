.global sqrt
.type sqrt, @function
sqrt:
    fldl 4(%esp)
    fsqrt
    ret
.size sqrt, .-sqrt

.global sqrtf
.type sqrtf, @function
sqrtf:
    flds 4(%esp)
    fsqrt
    ret
.size sqrtf, .-sqrtf
