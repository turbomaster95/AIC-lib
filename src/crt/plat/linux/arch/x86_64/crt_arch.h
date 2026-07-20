#ifndef _CRT_ARCH_H
#define _CRT_ARCH_H

__asm__(
".text\n"
".global " START "\n"
".type " START ", @function\n"
START ":\n"
"	xor %rbp, %rbp\n"
"	mov %rsp, %rdi\n"
"	andq $-16, %rsp\n"
"	call " START "_c\n"
);

#endif
