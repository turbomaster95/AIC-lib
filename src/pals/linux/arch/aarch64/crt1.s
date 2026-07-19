.section .text
.global _start

_start:
    mov x29, #0          // Frame pointer = 0 (End of stack trace)
    mov x30, #0          // Link register = 0
    
    mov x0, sp           // The Stack Pointer is our 1st argument (x0)
    bl _start_c          // Call our C entry point

_hang:
    b _hang              // Should never reach here
