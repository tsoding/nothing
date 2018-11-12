#include <assert.h>
#include <stdlib.h>

#include "game/level/platforms.h"
#include "physical_world.h"
#include "system/lt.h"
#include "system/nth_alloc.h"

#define PHYSICAL_WORLD_CAPACITY 256
#define PHYSICAL_WORLD_GRAVITY 1500.0f

struct Physical_world
{
    Lt *lt;
    size_t capacity;
    size_t size;
    Solid_ref *solids;
};

Physical_world *create_physical_world(void)
{
    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    Physical_world * const physical_world =
        PUSH_LT(lt, nth_alloc(sizeof(Physical_world)), free);
    if (physical_world == NULL) {
        RETURN_LT(lt, NULL);
    }

    physical_world->solids =
        PUSH_LT(
            lt,
            nth_alloc(sizeof(Solid_ref) * PHYSICAL_WORLD_CAPACITY),
            free);
    if (physical_world->solids == NULL) {
        RETURN_LT(lt, NULL);
    }

    physical_world->capacity = PHYSICAL_WORLD_CAPACITY;
    physical_world->size = 0;
    physical_world->lt = lt;

    return physical_world;
}

void destroy_physical_world(Physical_world *physical_world)
{
    assert(physical_world);
    RETURN_LT0(physical_world->lt);
}

void physical_world_apply_gravity(Physical_world *physical_world)
{
    for (size_t i = 0; i < physical_world->size; ++i) {
        solid_apply_force(
            physical_world->solids[i],
            vec(0.0f, PHYSICAL_WORLD_GRAVITY));
    }
}

void physical_world_collide_solids(Physical_world *physical_world,
                                   Platforms *platforms)
{
    assert(physical_world);

    for (size_t i = 0; i < physical_world->size; ++i) {
        solid_collide_with_solid(
            physical_world->solids[i],
            platforms_as_solid(platforms));

        for (size_t j = 0; j < physical_world->size; ++j) {
            if (i != j) {
                solid_collide_with_solid(
                    physical_world->solids[i],
                    physical_world->solids[j]);
            }
        }

        solid_collide_with_solid(
            physical_world->solids[i],
            platforms_as_solid(platforms));
    }
}

int physical_world_add_solid(Physical_world *physical_world,
                             Solid_ref solid)
{
    assert(physical_world);

    if (physical_world->size >= physical_world->capacity) {
        const size_t new_capacity = physical_world->capacity * 2;
        Solid_ref * const new_solids = nth_realloc(
            physical_world->solids,
            sizeof(Solid_ref) * new_capacity);

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

void physical_world_clean(Physical_world *physical_world)
{
    assert(physical_world);
    physical_world->size = 0;
}
