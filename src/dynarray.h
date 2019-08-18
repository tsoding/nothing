#ifndef DYNARRAY_H_
#define DYNARRAY_H_

#include <stdbool.h>

typedef struct Dynarray Dynarray;

Dynarray *create_dynarray(size_t element_size);
void destroy_dynarray(Dynarray *dynarray);

size_t dynarray_count(const Dynarray *dynarray);
void *dynarray_data(Dynarray *dynarray);
void *dynarray_pointer_at(Dynarray *dynarray, size_t index);
void dynarray_clear(Dynarray *dynarray);
// O(1) amortized
// TODO(#981): dynarray_push should be called dynarray_push_copy
int dynarray_push(Dynarray *dynarray, const void *element);
int dynarray_push_empty(Dynarray *dynarray);
void dynarray_pop(Dynarray *dynarray, void *element);
bool dynarray_contains(const Dynarray *dynarray,
                       const void *element);
// O(N)
void dynarray_delete_at(Dynarray *dynarray, size_t index);
void dynarray_insert_before(Dynarray *dynarray, size_t index, void *element);

#endif  // DYNARRAY_H_
