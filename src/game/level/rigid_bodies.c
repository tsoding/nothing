#include <stdlib.h>
#include <stdbool.h>

#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/stacktrace.h"
#include "game/camera.h"

#include "./rigid_bodies.h"

struct RigidBodies
{
    Lt *lt;
    size_t capacity;
    size_t count;

    Vec *positions;
    Vec *velocities;
    Vec *movements;
    Vec *sizes;
    Color *colors;
    bool *grounded;
    Vec *forces;
};

RigidBodies *create_rigid_bodies(size_t capacity)
{
    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    RigidBodies *rigid_bodies = PUSH_LT(lt, nth_calloc(1, sizeof(RigidBodies)), free);
    if (rigid_bodies == NULL) {
        RETURN_LT(lt, NULL);
    }
    rigid_bodies->lt = lt;

    rigid_bodies->capacity = capacity;
    rigid_bodies->count = 0;

    rigid_bodies->positions = PUSH_LT(lt, nth_calloc(capacity, sizeof(Vec)), free);
    if (rigid_bodies->positions == NULL) {
        RETURN_LT(lt, NULL);
    }

    rigid_bodies->velocities = PUSH_LT(lt, nth_calloc(capacity, sizeof(Vec)), free);
    if (rigid_bodies->velocities == NULL) {
        RETURN_LT(lt, NULL);
    }

    rigid_bodies->movements = PUSH_LT(lt, nth_calloc(capacity, sizeof(Vec)), free);
    if (rigid_bodies->movements == NULL) {
        RETURN_LT(lt, NULL);
    }

    rigid_bodies->sizes = PUSH_LT(lt, nth_calloc(capacity, sizeof(Vec)), free);
    if (rigid_bodies->sizes == NULL) {
        RETURN_LT(lt, NULL);
    }

    rigid_bodies->colors = PUSH_LT(lt, nth_calloc(capacity, sizeof(Color)), free);
    if (rigid_bodies->colors == NULL) {
        RETURN_LT(lt, NULL);
    }

    rigid_bodies->grounded = PUSH_LT(lt, nth_calloc(capacity, sizeof(bool)), free);
    if (rigid_bodies->grounded == NULL) {
        RETURN_LT(lt, NULL);
    }

    rigid_bodies->forces = PUSH_LT(lt, nth_calloc(capacity, sizeof(Vec)), free);
    if (rigid_bodies->forces == NULL) {
        RETURN_LT(lt, NULL);
    }

    return rigid_bodies;
}

void destroy_rigid_bodies(RigidBodies *rigid_bodies)
{
    trace_assert(rigid_bodies);
    RETURN_LT0(rigid_bodies->lt);
}

int rigid_bodies_update(RigidBodies *rigid_bodies,
                        float delta_time)
{
    trace_assert(rigid_bodies);
    (void) delta_time;
    /* TODO(#639): rigid_bodies_update is not implemented */
    return 0;
}

int rigid_bodies_render(RigidBodies *rigid_bodies,
                        Camera *camera)
{
    trace_assert(rigid_bodies);
    trace_assert(camera);

    for (size_t i = 0; i < rigid_bodies->count; ++i) {
        if (camera_fill_rect(
                camera,
                rect_from_vecs(
                    rigid_bodies->positions[i],
                    rigid_bodies->sizes[i]),
                rigid_bodies->colors[i]) < 0) {
            return -1;
        }
    }

    return 0;
}

RigidBodyId rigid_bodies_add(RigidBodies *rigid_bodies,
                             Rect rect,
                             Color color)
{
    trace_assert(rigid_bodies);
    trace_assert(rigid_bodies->count < rigid_bodies->capacity);

    RigidBodyId id = rigid_bodies->count++;
    rigid_bodies->positions[id] = vec(rect.x, rect.y);
    rigid_bodies->sizes[id] = vec(rect.w, rect.h);
    rigid_bodies->colors[id] = color;

    return id;
}

Rect rigid_bodies_hitbox(const RigidBodies *rigid_bodies,
                         RigidBodyId id)
{
    trace_assert(rigid_bodies);
    trace_assert(id < rigid_bodies->count);

    return rect_from_vecs(rigid_bodies->positions[id],
                          rigid_bodies->sizes[id]);
}

void rigid_bodies_move(RigidBodies *rigid_bodies,
                       RigidBodyId id,
                       Vec movement)
{
    trace_assert(rigid_bodies);
    trace_assert(id < rigid_bodies->count);

    rigid_bodies->movements[id] = movement;
}

int rigid_bodies_touches_ground(const RigidBodies *rigid_bodies,
                                RigidBodyId id)
{
    trace_assert(rigid_bodies);
    trace_assert(id < rigid_bodies->count);

    return rigid_bodies->grounded[id];
}

void rigid_bodies_apply_force(RigidBodies * rigid_bodies,
                              RigidBodyId id,
                              Vec force)
{
    trace_assert(rigid_bodies);
    trace_assert(id < rigid_bodies->count);

    rigid_bodies->forces[id] = vec_sum(rigid_bodies->forces[id], force);
}

void rigid_bodies_transform_velocity(RigidBodies *rigid_bodies,
                                     RigidBodyId id,
                                     mat3x3 trans_mat)
{
    trace_assert(rigid_bodies);
    trace_assert(id < rigid_bodies->count);

    rigid_bodies->velocities[id] = point_mat3x3_product(
        rigid_bodies->velocities[id],
        trans_mat);
}

void rigid_bodies_teleport_to(RigidBodies *rigid_bodies,
                              RigidBodyId id,
                              Vec position)
{
    trace_assert(rigid_bodies);
    trace_assert(id < rigid_bodies->count);

    rigid_bodies->positions[id] = position;
}

void rigid_bodies_damper(RigidBodies *rigid_bodies,
                         RigidBodyId id,
                         Vec v)
{
    trace_assert(rigid_bodies);
    trace_assert(id < rigid_bodies->count);

    rigid_bodies_apply_force(
        rigid_bodies, id,
        vec(
            rigid_bodies->velocities[id].x * v.x,
            rigid_bodies->velocities[id].y * v.y));
}
