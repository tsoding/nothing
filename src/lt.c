#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "./lt.h"
#include "./lt/lt_slot.h"

#define INITIAL_FRAME_BUFFER_SIZE 16

struct lt_t
{
    lt_slot_t **frames;
    size_t capacity;
    size_t size;
};

lt_t *create_lt()
{
    lt_t *lt = malloc(sizeof(lt_t));
    if(lt == NULL) {
        goto malloc_lt_fail;
    }

    lt->frames = malloc(sizeof(lt_slot_t*) * INITIAL_FRAME_BUFFER_SIZE);
    if (lt->frames == NULL) {
        goto malloc_lt_slots_fail;
    }

    lt->capacity = INITIAL_FRAME_BUFFER_SIZE;
    lt->size = 0;

    return lt;

malloc_lt_slots_fail:
    free(lt);
malloc_lt_fail:
    return NULL;
}

void destroy_lt(lt_t *lt)
{
    assert(lt);

    while (lt->size-- > 0) {
        destroy_lt_slot(lt->frames[lt->size]);
    }

    free(lt->frames);
    free(lt);
}

void *lt_push(lt_t *lt, void *resource, lt_destroy_t resource_destroy)
{
    assert(lt);
    assert(resource);
    assert(resource_destroy);

    if (lt->size >= lt->capacity) {
        lt->capacity *= 2;
        if ((lt->frames = realloc(lt->frames, lt->capacity)) == NULL) {
            return NULL;
        }
    }

    if ((lt->frames[lt->size++] = create_lt_slot(resource, resource_destroy)) == NULL) {
        return NULL;
    }

    return resource;
}

void* lt_reset(lt_t *lt, void *old_resource, void *new_resource)
{
    for (size_t i = 0; i < lt->size; ++i) {
        if (lt_slot_contains_resource(lt->frames[i], old_resource)) {
            lt_slot_reset_resource(lt->frames[i], new_resource);
            return new_resource;
        }
    }

    return old_resource;
}
