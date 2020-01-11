#ifndef DYNARRAY_H_
#define DYNARRAY_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "system/memory.h"
#include "system/stacktrace.h"

#define DYNARRAY_CAPACITY 256

typedef struct {
    size_t element_size;
    size_t count;
    void *data;
} Dynarray;

static inline
Dynarray create_dynarray_malloc(size_t element_size)
{
    Dynarray result = {
        .element_size = element_size,
        .count = 0,
        .data = malloc(DYNARRAY_CAPACITY * element_size)
    };
    trace_assert(result.data);
    return result;
}

static inline
Dynarray create_dynarray(Memory *memory, size_t element_size)
{
    trace_assert(memory);
    Dynarray result = {
        .element_size = element_size,
        .count = 0,
        .data = memory_alloc(memory, DYNARRAY_CAPACITY * element_size)
    };
    return result;
}

void *dynarray_pointer_at(const Dynarray *dynarray, size_t index);
void dynarray_replace_at(Dynarray *dynarray, size_t index, void *element);
void dynarray_copy_to(Dynarray *dynarray, void *dest, size_t index);
void dynarray_clear(Dynarray *dynarray);
// O(1) amortized
// TODO(#981): dynarray_push should be called dynarray_push_copy
int dynarray_push(Dynarray *dynarray, const void *element);
int dynarray_push_empty(Dynarray *dynarray);
void dynarray_pop(Dynarray *dynarray, void *element);
bool dynarray_contains(const Dynarray *dynarray,
                       const void *element);

void dynarray_swap(Dynarray *dynarray, size_t i, size_t j);

// O(N)
void dynarray_delete_at(Dynarray *dynarray, size_t index);
void dynarray_insert_before(Dynarray *dynarray, size_t index, void *element);

#endif  // DYNARRAY_H_
