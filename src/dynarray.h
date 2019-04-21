#ifndef DYNARRAY_H_
#define DYNARRAY_H_

#include <stdbool.h>

typedef struct Dynarray Dynarray;

Dynarray *create_dynarray(size_t element_size);
void destroy_dynarray(Dynarray *dynarray);

size_t dynarray_count(const Dynarray *dynarray);
void *dynarray_data(Dynarray *dynarray);
void dynarray_clear(Dynarray *dynarray);
int dynarray_push(Dynarray *dynarray, const void *element);
bool dynarray_contains(const Dynarray *dynarray,
                       const void *element);
void dynarray_delete_at(Dynarray *dynarray, size_t index);

#endif  // DYNARRAY_H_
