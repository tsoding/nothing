#include <stdlib.h>
#include "nth_alloc.h"
#include "log.h"

void *nth_calloc(size_t num, size_t size)
{
    void *mem = calloc(num, size);

    if (mem == NULL) {
        log_fail("nth_calloc(%lu, %lu) failed", num, size);
    }

    return mem;
}
