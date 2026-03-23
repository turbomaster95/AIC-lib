#include <stdlib.h>
#include <string.h>

void *calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    void *ptr = malloc(total);
    
    if (ptr) {
        memset(ptr, 0, total);
    }
    
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    if (!ptr) {
        return malloc(size);
    }
    
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    
    /* Get old size from header */
    size_t *header = (size_t *)((char *)ptr - sizeof(size_t));
    size_t old_size = *header - sizeof(size_t);
    
    /* Allocate new memory */
    void *new_ptr = malloc(size);
    if (!new_ptr) {
        return NULL;
    }
    
    /* Copy old data */
    size_t copy_size = (old_size < size) ? old_size : size;
    memcpy(new_ptr, ptr, copy_size);
    
    /* Free old memory */
    free(ptr);
    
    return new_ptr;
}
