.section .init
.global _init
.type _init, @function
_init:
	push %eax

.section .fini
.global _fini
.type _fini, @function
_fini:
	push %eax
