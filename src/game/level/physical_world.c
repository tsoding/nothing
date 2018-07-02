#include <assert.h>
#include <stdlib.h>

#include "physical_world.h"
#include "system/error.h"
#include "system/lt.h"

#define PHYSICAL_WORLD_CAPACITY 256

struct physical_world_t
{
    lt_t *lt;
    size_t capacity;
    size_t size;
    solid_ref_t *solids;
};

physical_world_t *create_physical_world(void)
{
    lt_t *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    physical_world_t * const physical_world =
        PUSH_LT(lt, malloc(sizeof(physical_world_t)), free);
    if (physical_world == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    physical_world->solids =
        PUSH_LT(
            lt,
            malloc(sizeof(solid_ref_t) * PHYSICAL_WORLD_CAPACITY),
            free);
    if (physical_world->solids == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    physical_world->capacity = PHYSICAL_WORLD_CAPACITY;
    physical_world->size = 0;
    physical_world->lt = lt;

    return physical_world;
}

void destroy_physical_world(physical_world_t *physical_world)
{
    assert(physical_world);
    RETURN_LT0(physical_world->lt);
}

void physical_world_collide_solids(physical_world_t *physical_world)
{
    assert(physical_world);
    for (size_t i = 0; i < physical_world->size; ++i) {
        for (size_t j = 0; j < physical_world->size; ++j) {
            if (i != j) {
                solid_collide_with_solid(
                    physical_world->solids[i],
                    physical_world->solids[j]);
            }
        }
    }
}

int physical_world_add_solid(physical_world_t *physical_world,
                             solid_ref_t solid)
{
    assert(physical_world);

    if (physical_world->size >= physical_world->capacity) {
        const size_t new_capacity = physical_world->capacity * 2;
        solid_ref_t * const new_solids = realloc(
            physical_world->solids,
            sizeof(solid_ref_t) * new_capacity);

        if (new_solids == NULL) {
            return -1;
        }

        physical_world->capacity = new_capacity;
        physical_world->solids = REPLACE_LT(
            physical_world->lt,
            physical_world->solids,
            new_solids);
    }

    physical_world->solids[physical_world->size++] = solid;

    return 0;
}
