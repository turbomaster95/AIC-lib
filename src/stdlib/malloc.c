#include <internal/syscall.h>
#include <stdlib.h>
#include <stdint.h>

static void *current_brk = 0;

/* Free block header - stored in the freed block itself */
typedef struct free_block {
    size_t size;
    struct free_block *next;
} free_block_t;

/* Global free list - shared with free.c */
free_block_t *free_list = 0;

void *malloc(size_t size) {
    /* Minimum allocation size (header + at least 1 byte, aligned) */
    const size_t min_size = sizeof(size_t) + 8;
    
    /* Align to 8 bytes (include header in alignment) */
    size_t aligned_size = (size + sizeof(size_t) + 7) & ~7;
    
    /* First, try to find a suitable free block */
    free_block_t **prev = &free_list;
    free_block_t *block = free_list;
    
    while (block) {
        if (block->size >= aligned_size) {
            /* Found a suitable block */
            size_t remaining = block->size - aligned_size;
            
            /* If remaining space is too small, use the whole block */
            if (remaining < min_size) {
                *prev = block->next;
                return (void *)((char *)block + sizeof(size_t));
            }
            
            /* Split the block */
            free_block_t *new_block = (free_block_t *)((char *)block + aligned_size);
            new_block->size = remaining;
            new_block->next = block->next;
            *prev = new_block;
            
            block->size = aligned_size;
            return (void *)((char *)block + sizeof(size_t));
        }
        prev = &block->next;
        block = block->next;
    }
    
    /* No free block found, allocate new memory from brk */
    if (current_brk == 0) {
        current_brk = (void *)__syscall3(SYS_brk, 0, 0, 0);
    }
    
    void *ptr = current_brk;
    uintptr_t new_brk = (uintptr_t)current_brk + aligned_size;
    
    if (__syscall3(SYS_brk, new_brk, 0, 0) != -1) {
        /* Store the size in the header */
        *(size_t *)ptr = aligned_size;
        current_brk = (void *)new_brk;
        
        /* Return the address AFTER the header */
        return (void *)((char *)ptr + sizeof(size_t));
    }
    
    return 0;
}
