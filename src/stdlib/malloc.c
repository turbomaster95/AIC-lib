#include <internal/syscall.h>
#include <stdlib.h>

static void *current_brk = 0;

void *malloc(size_t size) {
    if (current_brk == 0) {
        current_brk = (void *)__syscall3(SYS_brk, 0, 0, 0);
    }

    // 1. Add space for an 8-byte header
    // 2. Align the total size to 8 bytes
    size_t total_size = (size + 8 + 7) & ~7;
    
    void *ptr = current_brk;
    unsigned long new_brk = (unsigned long)current_brk + total_size;
    
    if (__syscall3(SYS_brk, new_brk, 0, 0) != -1) {
        // Store the size in the header
        *(size_t *)ptr = total_size;
        
        current_brk = (void *)new_brk;
        
        // Return the address AFTER the header
        return (void *)((char *)ptr + 8);
    }
    return 0; 
}
