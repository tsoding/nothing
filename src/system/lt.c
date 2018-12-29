#include "system/stacktrace.h"
#include <stdio.h>
#include <stdlib.h>

#include "lt.h"
#include "lt/lt_slot.h"
#include "system/nth_alloc.h"

#define INITIAL_FRAME_BUFFER_SIZE 16

struct Lt
{
    Lt_slot **frames;
    size_t capacity;
    size_t size;
};

Lt *create_lt()
{
    Lt *lt = nth_alloc(sizeof(Lt));
    if(lt == NULL) {
        goto nth_alloc_lt_fail;
    }

    lt->frames = nth_alloc(sizeof(Lt_slot*) * INITIAL_FRAME_BUFFER_SIZE);
    if (lt->frames == NULL) {
        goto nth_alloc_lt_slots_fail;
    }

    lt->capacity = INITIAL_FRAME_BUFFER_SIZE;
    lt->size = 0;

    return lt;

nth_alloc_lt_slots_fail:
    free(lt);
nth_alloc_lt_fail:
    return NULL;
}

void destroy_lt(Lt *lt)
{
    trace_assert(lt);

    while (lt->size-- > 0) {
        if (lt->frames[lt->size]) {
            destroy_lt_slot(lt->frames[lt->size]);
        }
    }

    free(lt->frames);
    free(lt);
}

void *lt_push(Lt *lt, void *resource, Lt_destroy resource_destroy)
{
    trace_assert(lt);
    trace_assert(resource_destroy);
    trace_assert(lt != resource);

    if (resource == NULL) {
        return NULL;
    }

    if (lt->size >= lt->capacity) {
        lt->capacity *= 2;
        if ((lt->frames = nth_realloc(lt->frames, sizeof(Lt_slot*) * lt->capacity)) == NULL) {
            return NULL;
        }
    }

    if ((lt->frames[lt->size++] = create_lt_slot(resource, resource_destroy)) == NULL) {
        return NULL;
    }

    return resource;
}

void* lt_reset(Lt *lt, void *old_resource, void *new_resource)
{
    trace_assert(lt);
    trace_assert(old_resource);
    trace_assert(new_resource);
    trace_assert(old_resource != new_resource);

    for (size_t i = 0; i < lt->size; ++i) {
        if (lt->frames[i] && lt_slot_contains_resource(lt->frames[i], old_resource)) {
            lt_slot_reset_resource(lt->frames[i], new_resource);
            return new_resource;
        }
    }

    return old_resource;
}

void *lt_release(Lt *lt, void *resource)
{
    trace_assert(lt);
    trace_assert(resource);

    for (size_t i = 0; i < lt->size; ++i) {
        if (lt->frames[i] && lt_slot_contains_resource(lt->frames[i], resource)) {
            release_lt_slot(lt->frames[i]);
            lt->frames[i] = NULL;
            return resource;
        }
    }

    return resource;
}

void *lt_replace(Lt *lt, void *old_resource, void *new_resource)
{
    trace_assert(lt);
    trace_assert(old_resource);
    trace_assert(new_resource);

    for (size_t i = 0; i < lt->size; ++i) {
        if (lt->frames[i] && lt_slot_contains_resource(lt->frames[i], old_resource)) {
            lt_slot_replace_resource(lt->frames[i], new_resource);
            return new_resource;
        }
    }

    return old_resource;
}
