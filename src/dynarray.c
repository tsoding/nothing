#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "system/stacktrace.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "dynarray.h"

void *dynarray_pointer_at(const Dynarray *dynarray, size_t index)
{
    trace_assert(index < dynarray->count);
    return (uint8_t *)dynarray->data + index * dynarray->element_size;
}

void dynarray_clear(Dynarray *dynarray)
{
    trace_assert(dynarray);
    dynarray->count = 0;
}

int dynarray_push(Dynarray *dynarray, const void *element)
{
    trace_assert(dynarray);
    trace_assert(element);
    trace_assert(dynarray->count < DYNARRAY_CAPACITY);

    memcpy(
        (char*) dynarray->data + dynarray->count * dynarray->element_size,
        element,
        dynarray->element_size);

    dynarray->count++;

    return 0;
}

bool dynarray_contains(const Dynarray *dynarray,
                       const void *element)
{
    trace_assert(dynarray);
    trace_assert(element);

    for (size_t i = 0; i < dynarray->count; ++i) {
        if (memcmp((const char*)dynarray->data + i * dynarray->element_size,
                   (const char*)element,
                   dynarray->element_size) == 0) {
            return true;
        }
    }

    return false;
}

void dynarray_delete_at(Dynarray *dynarray, size_t index)
{
    trace_assert(dynarray);
    trace_assert(index < dynarray->count);
    memmove(
        (uint8_t *) dynarray->data + index * dynarray->element_size,
        (uint8_t *) dynarray->data + (index + 1) * dynarray->element_size,
        dynarray->element_size * (dynarray->count - index - 1));
    dynarray->count--;
}

void dynarray_insert_before(Dynarray *dynarray, size_t index, void *element)
{
    trace_assert(dynarray);
    trace_assert(dynarray->count < DYNARRAY_CAPACITY);
    trace_assert(element);
    trace_assert(index <= dynarray->count);

    if (index == dynarray->count) {
        dynarray_push(dynarray, element);
        return;
    }

    memmove(
        (uint8_t*) dynarray->data + (index + 1) * dynarray->element_size,
        (uint8_t*) dynarray->data + index * dynarray->element_size,
        dynarray->element_size * (dynarray->count - index));

    memcpy(
        (uint8_t*) dynarray->data + index * dynarray->element_size,
        element,
        dynarray->element_size);

    dynarray->count++;
}

int dynarray_push_empty(Dynarray *dynarray)
{
    trace_assert(dynarray);
    trace_assert(dynarray->count < DYNARRAY_CAPACITY);

    memset(
        (char*) dynarray->data + dynarray->count * dynarray->element_size,
        0,
        dynarray->element_size);

    dynarray->count++;

    return 0;
}

// TODO(#980): dynarray_push and dynarray_push_empty have duplicate codez

void dynarray_pop(Dynarray *dynarray, void *element)
{
    trace_assert(dynarray);
    trace_assert(dynarray->count > 0);

    dynarray->count--;

    if (element) {
        memcpy(
            element,
            (uint8_t*) dynarray->data + dynarray->count * dynarray->element_size,
            dynarray->element_size);
    }
}

void dynarray_replace_at(Dynarray *dynarray, size_t index, void *element)
{
    trace_assert(dynarray);
    trace_assert(element);
    trace_assert(index < dynarray->count);

    memcpy(
        (uint8_t*) dynarray->data + index * dynarray->element_size,
        element,
        dynarray->element_size);
}

void dynarray_copy_to(Dynarray *dynarray, void *dest, size_t index)
{
    trace_assert(dynarray);
    trace_assert(dest);
    trace_assert(index < dynarray->count);

    memcpy(dest, (uint8_t*) dynarray->data + index * dynarray->element_size, dynarray->element_size);
}

void dynarray_swap(Dynarray *dynarray, size_t i, size_t j)
{
    trace_assert(dynarray);
    trace_assert(i < dynarray->count);
    trace_assert(j < dynarray->count);

    if (i == j) return;

    char *a = dynarray_pointer_at(dynarray, i);
    char *b = dynarray_pointer_at(dynarray, j);

    for (size_t k = 0; k < dynarray->element_size; ++k) {
        char t = a[k];
        a[k] = b[k];
        b[k] = t;
    }
}
