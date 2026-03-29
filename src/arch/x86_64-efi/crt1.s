# src/arch/x86_64-efi/crt1.s
.section .text
.global _start

_start:
    xor %rbp, %rbp      # Clear frame pointer (end of stack trace)
    
    # In our bios-nim OS, we don't have a complex stack setup yet. 
    # We'll assume %rsp is already at a valid location.
    mov %rsp, %rdi      # Pass current stack pointer to _start_c
    
    # Align the stack to 16 bytes for C
    and $-16, %rsp
    
    call _start_c       # Call AIC's C entry point (calls main)
    
    # At this point, RAX contains the return value of main.
    # We pass it as the first argument (RDI) to exit().
    mov %rax, %rdi
    call exit           # Call the exit syscall wrapper
    
_hang:                  # Fallback in case exit fails
    jmp _hang
