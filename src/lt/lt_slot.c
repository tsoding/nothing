#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../error.h"
#include "./lt_slot.h"

struct lt_slot_t
{
    void *resource;
    lt_destroy_t resource_destroy;
};

lt_slot_t *create_lt_slot(void *resource, lt_destroy_t resource_destroy)
{
    assert(resource);
    assert(resource_destroy);

    lt_slot_t *lt_slot = malloc(sizeof(lt_slot_t));
    if (lt_slot == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        return NULL;
    }

    lt_slot->resource = resource;
    lt_slot->resource_destroy = resource_destroy;

    return lt_slot;
}

void *release_lt_slot(lt_slot_t *lt_slot)
{
    assert(lt_slot);

    void *resource = lt_slot->resource;
    free(lt_slot);
    return resource;
}

void destroy_lt_slot(lt_slot_t *lt_slot)
{
    assert(lt_slot);

    lt_slot->resource_destroy(lt_slot->resource);
    free(lt_slot);
}

void lt_slot_reset_resource(lt_slot_t *lt_slot, void *resource)
{
    assert(lt_slot);
    assert(resource);

    lt_slot->resource_destroy(lt_slot->resource);
    lt_slot->resource = resource;
}

int lt_slot_contains_resource(const lt_slot_t *lt_slot, void *resource)
{
    assert(lt_slot);
    assert(resource);

    return lt_slot->resource == resource;
}
