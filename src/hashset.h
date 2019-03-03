#ifndef HASHSET_H_
#define HASHSET_H_

#include <stdbool.h>

typedef struct HashSet HashSet;
typedef struct HashSetIterator HashSetIterator;

HashSet *create_hashset(size_t element_size, size_t n);
void destroy_hashset(HashSet *hashset);

int hashset_insert(HashSet *hashset, const void *element);
int hashset_remove(HashSet *hashset, const void *element);
bool hashset_contains(HashSet *hashset, const void *element);
void hashset_clear(HashSet *hashset);
size_t hashset_count(HashSet *hashset);

void *hashset_values(HashSet *hashset);

#endif  // HASHSET_H_
