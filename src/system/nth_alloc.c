#include <stdlib.h>
#include "nth_alloc.h"
#include "log.h"

/* TODO(#477): all instances of malloc should be replaced with nth_alloc (except ebisp) */
void *nth_alloc(size_t size)
{
    void *mem = malloc(size);

    if (mem == NULL) {
        log_error("nth_alloc(%lu) failed", size);
    }

    return mem;
}

/* TODO(#478): all instances of calloc should be replaced with nth_calloc (except ebisp) */
void *nth_calloc(size_t num, size_t size)
{
    void *mem = calloc(num, size);

    if (mem == NULL) {
        log_error("nth_calloc(%lu, %lu) failed", num, size);
    }

    return mem;
}
