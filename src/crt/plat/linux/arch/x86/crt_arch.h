#ifndef _CRT_ARCH_H
#define _CRT_ARCH_H

__asm__(
".text\n"
".global " START "\n"
".type " START ", @function\n"
START ":\n"
"       xor %ebp, %ebp\n"
"       pop %eax\n"
"       mov %esp, %ecx\n"
"       and $-16, %esp\n"
"       push %eax\n"
"       push %ecx\n"
"       call " START "_c\n"
);

#endif

