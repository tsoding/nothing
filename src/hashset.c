#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "system/lt.h"
#include "system/stacktrace.h"
#include "system/nth_alloc.h"
#include "hashset.h"
#include "dynarray.h"

struct HashSet
{
    Lt lt;
    size_t n;
    size_t element_size;
    size_t count;
    Dynarray **buckets;
    Dynarray *view;
};

static uint64_t fnv1(const char *data, size_t size)
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
    Lt lt = {0};

    HashSet *hash_set = PUSH_LT(lt, nth_calloc(1, sizeof(HashSet)), free);
    if (hash_set == NULL) {
        RETURN_LT(lt, NULL);
    }
    hash_set->lt = lt;

    hash_set->n = n;
    hash_set->count = 0;
    hash_set->element_size = element_size;

    hash_set->buckets = PUSH_LT(lt, nth_calloc(n, sizeof(Dynarray*)), free);
    if (hash_set->buckets == NULL) {
        RETURN_LT(lt, NULL);
    }

    for (size_t i = 0; i < n; ++i) {
        hash_set->buckets[i] = PUSH_LT(
            lt,
            create_dynarray(element_size),
            destroy_dynarray);
        if (hash_set->buckets[i] == NULL) {
            RETURN_LT(lt, NULL);
        }
    }

    hash_set->view = PUSH_LT(lt, create_dynarray(element_size), destroy_dynarray);
    if (hash_set->view == NULL) {
        RETURN_LT(lt, NULL);
    }

    return hash_set;
}

void destroy_hashset(HashSet *hashset)
{
    trace_assert(hashset);
    RETURN_LT0(hashset->lt);
}

int hashset_insert(HashSet *hashset, const void *element)
{
    trace_assert(hashset);
    trace_assert(element);

    const uint64_t hash = fnv1(element, hashset->element_size);
    const size_t i = hash % hashset->n;

    if (!dynarray_contains(hashset->buckets[i], element)) {
        dynarray_push(hashset->buckets[i], element);
        hashset->count++;
    }

    return 0;
}

bool hashset_contains(HashSet *hashset, const void *element)
{
    trace_assert(hashset);
    trace_assert(element);

    const uint64_t hash = fnv1(element, hashset->element_size);
    const size_t i = hash % hashset->n;

    return dynarray_contains(hashset->buckets[i], element);
}

void hashset_clear(HashSet *hashset)
{
    trace_assert(hashset);

    for (size_t i = 0; i < hashset->n; ++i) {
        dynarray_clear(hashset->buckets[i]);
    }

    hashset->count = 0;
}
size_t hashset_count(HashSet *hashset)

{
    trace_assert(hashset);
    return hashset->count;
}

void *hashset_values(HashSet *hashset)
{
    dynarray_clear(hashset->view);

    for (size_t i = 0; i < hashset->n; ++i) {
        size_t n = dynarray_count(hashset->buckets[i]);
        char *bucket = dynarray_data(hashset->buckets[i]);

        for (size_t j = 0; j < n; ++j) {
            dynarray_push(hashset->view, bucket + j * hashset->element_size);
        }
    }

    return dynarray_data(hashset->view);
}
