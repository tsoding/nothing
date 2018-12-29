#include "system/stacktrace.h"
#include <stdio.h>
#include <stdlib.h>

#include "lt_slot.h"
#include "system/nth_alloc.h"

struct Lt_slot
{
    void *resource;
    Lt_destroy resource_destroy;
};

Lt_slot *create_lt_slot(void *resource, Lt_destroy resource_destroy)
{
    trace_assert(resource);
    trace_assert(resource_destroy);

    Lt_slot *lt_slot = nth_alloc(sizeof(Lt_slot));
    if (lt_slot == NULL) {
        return NULL;
    }

    lt_slot->resource = resource;
    lt_slot->resource_destroy = resource_destroy;

    return lt_slot;
}

void *release_lt_slot(Lt_slot *lt_slot)
{
    trace_assert(lt_slot);

    void *resource = lt_slot->resource;
    free(lt_slot);
    return resource;
}

void destroy_lt_slot(Lt_slot *lt_slot)
{
    trace_assert(lt_slot);

    lt_slot->resource_destroy(lt_slot->resource);
    free(lt_slot);
}

void lt_slot_reset_resource(Lt_slot *lt_slot, void *resource)
{
    trace_assert(lt_slot);
    trace_assert(resource);

    lt_slot->resource_destroy(lt_slot->resource);
    lt_slot->resource = resource;
}

void lt_slot_replace_resource(Lt_slot *lt_slot, void *resource)
{
    trace_assert(lt_slot);
    trace_assert(resource);

    lt_slot->resource = resource;
}

int lt_slot_contains_resource(const Lt_slot *lt_slot, void *resource)
{
    trace_assert(lt_slot);
    trace_assert(resource);

    return lt_slot->resource == resource;
}
