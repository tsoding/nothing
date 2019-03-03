#ifndef HASHSET_H_
#define HASHSET_H_

typedef struct HashSet HashSet;

HashSet *create_hashset(size_t element_size, size_t n);
void destroy_hashset(HashSet *hashset);

int hashset_insert(HashSet *hashset, void *element);
int hashset_remove(HashSet *hashset, void *element);
bool hashset_contains(HashSet *hashset, void *element);
void hashset_clear(HashSet *hashset);

#endif  // HASHSET_H_
