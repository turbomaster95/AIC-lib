#include <stdlib.h>
#include <stdint.h>

/* Free block header - must match malloc.c */
typedef struct free_block {
    size_t size;
    struct free_block *next;
} free_block_t;

/* External reference to free_list from malloc.c */
extern free_block_t *free_list;

void free(void *ptr) {
    if (!ptr) return;
    
    /* Get the header (stored just before the user pointer) */
    size_t *header = (size_t *)((char *)ptr - sizeof(size_t));
    size_t size = *header;
    
    /* Add to free list */
    free_block_t *block = (free_block_t *)header;
    block->size = size;
    block->next = free_list;
    free_list = block;
}
