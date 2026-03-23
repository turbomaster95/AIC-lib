#include <errno.h>

/* 
 * Global errno - in a single-threaded or freestanding environment,
 * this simple global is sufficient. For multi-threaded support,
 * this would need to be thread-local storage (TLS).
 */
int errno = 0;
