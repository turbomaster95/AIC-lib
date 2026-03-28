.code32 
.text
.global setjmp
.global longjmp
.type setjmp, @function
.type longjmp, @function

setjmp:
    movl    4(%esp), %eax          # eax = jmp_buf pointer
    movl    %ebx, 0(%eax)
    movl    %ebp, 4(%eax)
    movl    %esi, 8(%eax)
    movl    %edi, 12(%eax)

    # Save the ESP as it should be AFTER setjmp returns
    leal    4(%esp), %edx          
    movl    %edx, 16(%eax)         

    # Save the return address
    movl    (%esp), %edx           
    movl    %edx, 20(%eax)         

    pushfl
    popl    %edx
    movl    %edx, 24(%eax)         

    xorl    %eax, %eax
    ret

longjmp:
    movl    4(%esp), %edx          # edx = jmp_buf pointer
    movl    8(%esp), %eax          # eax = val

    # If val is 0, make it 1
    testl   %eax, %eax
    jnz     .Lskip_zero
    incl    %eax
.Lskip_zero:

    # Restore registers from edx (the jmp_buf)
    movl    0(%edx), %ebx
    movl    4(%edx), %ebp
    movl    8(%edx), %esi
    movl    12(%edx), %edi

    # Restore eflags
    movl    24(%edx), %ecx
    pushl   %ecx
    popfl

    # IMPORTANT: Restore return address and ESP
    movl    20(%edx), %ecx         # ecx = saved EIP
    movl    16(%edx), %esp         # Restore ESP
    
    # Push the return address back onto the RESTORED stack
    pushl   %ecx
    ret
