# src/arch/x86_64/crt1.s
.section .text
.global _start

_start:
    xor %rbp, %rbp      # Clear frame pointer (end of stack trace)
    mov %rsp, %rdi      # x86_64 ABI: 1st argument is passed in %rdi
                        # We pass the current stack pointer to _start_c
    
    # Align the stack to 16 bytes before calling C
    # The CPU pushes nothing before _start, so RSP is already 8-byte aligned.
    # We subtract 8 to make it 16-byte aligned for the call.
    and $-16, %rsp
    
    call _start_c       # Call our C entry point

_hang:
    hlt                 # Halt the CPU if we ever return
    jmp _hang
