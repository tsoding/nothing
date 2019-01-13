#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "system/stacktrace.h"
#include "system/nth_alloc.h"
#include "dynarray.h"

#define DYNARRAY_INIT_CAPACITY 64

struct DynArrayMeta
{
    size_t size;
    size_t count;
    size_t capacity;
};

struct DynArray
{
    struct DynArrayMeta meta;
    char data[];
};

static inline struct DynArray *dynarray_from_data(void *data)
{
    return (struct DynArray*)((char*) data - sizeof(struct DynArrayMeta));
}

void *create_dynarray(size_t size)
{
    struct DynArray *dynarray =
        nth_alloc(sizeof(struct DynArray) + size * DYNARRAY_INIT_CAPACITY);

    dynarray->meta.size = size;
    dynarray->meta.count = 0;
    dynarray->meta.capacity = DYNARRAY_INIT_CAPACITY;

    return dynarray->data;
}

void destroy_dynarray(void *data)
{
    trace_assert(data);
    struct DynArray *dynarray = dynarray_from_data(data);
    free(dynarray);
}

size_t dynarray_count(void *data)
{
    trace_assert(data);
    struct DynArray *dynarray = dynarray_from_data(data);
    return dynarray->meta.count;
}

void *dynarray_push(void *data, void *elem)
{
    trace_assert(data);
    trace_assert(elem);

    struct DynArray *dynarray = dynarray_from_data(data);

    if (dynarray->meta.count >= dynarray->meta.capacity) {
        const size_t new_capacity = dynarray->meta.capacity * 2;
        struct DynArray * const new_dynarray =
            nth_realloc(dynarray, sizeof(struct DynArray) + dynarray->meta.size * new_capacity);

        if (new_dynarray == NULL) {
            return NULL;
        }

        dynarray = new_dynarray;
        dynarray->meta.capacity = new_capacity;
    }

    memcpy(
        dynarray->data + dynarray->meta.count * dynarray->meta.size,
        elem,
        dynarray->meta.size);
    dynarray->meta.count++;

    return dynarray->data;
}
