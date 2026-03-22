#include <stdlib.h>

void free(void *ptr) {
    if (!ptr) return;
    // In a real allocator, we would add this block to a 'Free List'
    // For now, AIC just stays lean.
}
