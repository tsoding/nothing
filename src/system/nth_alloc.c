#include <stdlib.h>
#include "nth_alloc.h"
#include "log.h"

/* TODO(#477): all instances of malloc should be replaced with nth_alloc (except ebisp) */
void *nth_alloc(size_t size)
{
    void *mem = malloc(size);

    if (mem == NULL) {
        log_fail("nth_alloc(%lu) failed", size);
    }

    return mem;
}

void *nth_calloc(size_t num, size_t size)
{
    void *mem = calloc(num, size);

    if (mem == NULL) {
        log_fail("nth_calloc(%lu, %lu) failed", num, size);
    }

    return mem;
}

void *nth_realloc(void *ptr, size_t new_size)
{
    void *mem = realloc(ptr, new_size);

    if (mem == NULL) {
        log_fail("nth_realloc(0x%x, %lu) failed", ptr, new_size);
    }

    return mem;
}
