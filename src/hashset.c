#include <stdlib.h>
#include <stdint.h>

#include "system/lt.h"
#include "system/stacktrace.h"
#include "system/nth_alloc.h"
#include "linked_list.h"
#include "hashset.h"

struct HashSet
{
    Lt *lt;
    size_t n;
    size_t element_size;
    LinkedList **buckets;
};

static uint64_t fnv1(char *data, size_t size)
{
    uint64_t hash = 0xcbf29ce484222325;

    for (size_t i = 0; i < size; ++i) {
        hash = hash * 0x100000001b3;
        hash = hash ^ (uint64_t) data[i];
    }

    return hash;
}

HashSet *create_hashset(size_t element_size, size_t n)
{
    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    HashSet *hash_set = PUSH_LT(lt, nth_alloc(sizeof(HashSet)), free);
    if (hash_set == NULL) {
        RETURN_LT(lt, NULL);
    }
    hash_set->lt = lt;

    hash_set->n = n;
    hash_set->element_size = element_size;

    hash_set->buckets = PUSH_LT(lt, nth_calloc(n, sizeof(LinkedList*)), free);
    if (hash_set->buckets == NULL) {
        RETURN_LT(lt, NULL);
    }

    for (size_t i = 0; i < n; ++i) {
        hash_set->buckets[i] = PUSH_LT(
            lt,
            create_linked_list(element_size),
            destroy_linked_list);
        if (hash_set->buckets[i] == NULL) {
            RETURN_LT(lt, NULL);
        }
    }

    return hash_set;
}

void destroy_hashset(HashSet *hashset)
{
    trace_assert(hashset);
    RETURN_LT0(hashset->lt);
}

int hashset_insert(HashSet *hashset, void *element)
{
    trace_assert(hashset);
    trace_assert(element);

    const uint64_t hash = fnv1(element, hashset->element_size);
    const size_t i = hash % hashset->n;

    if (linked_list_find(hashset->buckets[i], element) == NULL) {
        linked_list_push_back(hashset->buckets[i], element);
    }

    return 0;
}

int hashset_remove(HashSet *hashset, void *element)
{
    trace_assert(hashset);
    trace_assert(element);

    const uint64_t hash = fnv1(element, hashset->element_size);
    const size_t i = hash % hashset->n;

    NodeLL *node = linked_list_find(hashset->buckets[i], element);

    if (node != NULL) {
        linked_list_remove(hashset->buckets[i], node);
    }

    return 0;
}

bool hashset_contains(HashSet *hashset, void *element)
{
    trace_assert(hashset);
    trace_assert(element);

    const uint64_t hash = fnv1(element, hashset->element_size);
    const size_t i = hash % hashset->n;

    return linked_list_find(hashset->buckets[i], element) != NULL;
}

void hashset_clear(HashSet *hashset)
{
    trace_assert(hashset);

    for (size_t i = 0; i < hashset->n; ++i) {
        linked_list_clear(hashset->buckets[i]);
    }
}
}
