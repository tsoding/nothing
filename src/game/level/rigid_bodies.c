#include <stdlib.h>

#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/stacktrace.h"
#include "game/camera.h"
#include "algo/dynarray.h"

#include "./rigid_bodies.h"

/* TODO(#635): RigidBodies doesn't have enough functionality to replace Rigid_rect */

struct RigidBodies
{
    Vec *positions;
    Vec *velocities;
    Vec *movements;
    Vec *sizes;
    Color *colors;
    int *ground_touches;
    Vec *forces;
};

RigidBodies *create_rigid_bodies(void)
{
    RigidBodies *rigid_bodies = nth_calloc(1, sizeof(RigidBodies));
    if (rigid_bodies == NULL) {
        goto fail;
    }

    rigid_bodies->positions = create_dynarray(sizeof(Vec));
    if (rigid_bodies->positions == NULL) {
        goto fail;
    }

    rigid_bodies->velocities = create_dynarray(sizeof(Vec));
    if (rigid_bodies->velocities == NULL) {
        goto fail;
    }

    rigid_bodies->movements = create_dynarray(sizeof(Vec));
    if (rigid_bodies->movements == NULL) {
        goto fail;
    }

    rigid_bodies->sizes = create_dynarray(sizeof(Vec));
    if (rigid_bodies->sizes == NULL) {
        goto fail;
    }

    rigid_bodies->colors = create_dynarray(sizeof(Color));
    if (rigid_bodies->colors == NULL) {
        goto fail;
    }

    rigid_bodies->ground_touches = create_dynarray(sizeof(int));
    if (rigid_bodies->ground_touches == NULL) {
        goto fail;
    }

    rigid_bodies->forces = create_dynarray(sizeof(Vec));
    if (rigid_bodies->forces == NULL) {
        goto fail;
    }

    return rigid_bodies;

fail:
    destroy_rigid_bodies(rigid_bodies);
    return NULL;
}

void destroy_rigid_bodies(RigidBodies *rigid_bodies)
{
    if (rigid_bodies == NULL) {
        return;
    }

    if (rigid_bodies->positions != NULL) {
        destroy_dynarray(rigid_bodies->positions);
    }

    if (rigid_bodies->velocities != NULL) {
        destroy_dynarray(rigid_bodies->velocities);
    }

    if (rigid_bodies->movements != NULL) {
        destroy_dynarray(rigid_bodies->movements);
    }

    if (rigid_bodies->sizes != NULL) {
        destroy_dynarray(rigid_bodies->sizes);
    }

    if (rigid_bodies->colors != NULL) {
        destroy_dynarray(rigid_bodies->colors);
    }

    if (rigid_bodies->ground_touches != NULL) {
        destroy_dynarray(rigid_bodies->ground_touches);
    }

    if (rigid_bodies->forces != NULL) {
        destroy_dynarray(rigid_bodies->forces);
    }

    free(rigid_bodies);
}

int rigid_bodies_update(RigidBodies *rigid_bodies,
                        float delta_time)
{
    trace_assert(rigid_bodies);
    (void) delta_time;
    return 0;
}

int rigid_bodies_render(RigidBodies *rigid_bodies,
                        Camera *camera)
{
    trace_assert(rigid_bodies);
    trace_assert(camera);
    return 0;
}

int rigid_bodies_add(RigidBodies *rigid_bodies,
                     Rect rect,
                     Color color,
                     unsigned int *id)
{
    trace_assert(rigid_bodies);
    trace_assert(id);
    (void) rect;
    (void) color;
    return 0;
}

int rigid_bodies_remove(RigidBodies *rigid_bodies,
                        unsigned int id)
{
    trace_assert(rigid_bodies);
    (void) id;
    return 0;
}
