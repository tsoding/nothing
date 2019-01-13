#include <stdlib.h>

#include "system/stacktrace.h"
#include "dynarray.h"

void *create_dynarray(size_t size)
{
    (void) size;
    return NULL;
}

void destroy_dynarray(void *dynarray)
{
    trace_assert(dynarray);
}

size_t dynarray_count(void *dynarray)
{
    trace_assert(dynarray);
    return 0;
}

int dynarray_push(void *dynarray, void *elem)
{
    trace_assert(dynarray);
    trace_assert(elem);
    return 0;
}
