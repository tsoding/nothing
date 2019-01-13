#ifndef DYNARRAY_H_
#define DYNARRAY_H_

void *create_dynarray(size_t size);
void destroy_dynarray(void *dynarray);

size_t dynarray_count(void *dynarray);
void *dynarray_push(void *dynarray, void *elem);

#endif  // DYNARRAY_H_
