#ifndef _CRT_ARCH_H
#define _CRT_ARCH_H

__asm__(
".text\n"
".global " START "\n"
".type " START ", @function\n"
START ":\n"
"       mov x29, #0\n"
"       mov x30, #0\n"
"       mov x0, sp\n"
"       and sp, x0, #-16\n"
"       bl " START "_c\n"
);

#endif

