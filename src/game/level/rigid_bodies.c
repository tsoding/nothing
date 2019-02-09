#include <stdlib.h>
#include <stdbool.h>

#include "game/camera.h"
#include "game/level/platforms.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/stacktrace.h"

#include "./rigid_bodies.h"

#define ID_SIZE 100

struct RigidBodies
{
    Lt *lt;
    size_t capacity;
    size_t count;

    Rect *bodies;
    Vec *velocities;
    Vec *movements;
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

    rigid_bodies->bodies = PUSH_LT(lt, nth_calloc(capacity, sizeof(Rect)), free);
    if (rigid_bodies->bodies == NULL) {
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

static int rigid_bodies_collide_with_itself(RigidBodies *rigid_bodies)
{
    trace_assert(rigid_bodies);

    if (rigid_bodies->count == 0) {
        return 0;
    }

    for (size_t i1 = 0; i1 < rigid_bodies->count - 1; ++i1) {
        for (size_t i2 = i1 + 1; i2 < rigid_bodies->count; ++i2) {
            // TODO(#654): Rigid Bodies don't exchange forces with each other
            if (!rects_overlap(rigid_bodies->bodies[i1], rigid_bodies->bodies[i2])) {
                continue;
            }

            const Vec orient = rect_impulse(&rigid_bodies->bodies[i1], &rigid_bodies->bodies[i2]);
            const Vec v = vec_sub(vec(1.0f, 1.0f), orient);

            rigid_bodies->velocities[i1] = vec_entry_mult(rigid_bodies->velocities[i1], v);
            rigid_bodies->velocities[i2] = vec_entry_mult(rigid_bodies->velocities[i2], v);
        }
    }

    return 0;
}

static int rigid_bodies_collide_with_platforms(
    RigidBodies *rigid_bodies,
    const Platforms *platforms)
{
    trace_assert(rigid_bodies);
    trace_assert(platforms);

    int sides[RECT_SIDE_N] = { 0, 0, 0, 0 };

    for (size_t i = 0; i < rigid_bodies->count; ++i) {
        memset(sides, 0, sizeof(int) * RECT_SIDE_N);

        platforms_touches_rect_sides(platforms, rigid_bodies->bodies[i], sides);

        if (sides[RECT_SIDE_BOTTOM]) {
            rigid_bodies->grounded[i] = true;
        }

        Vec v = platforms_snap_rect(platforms, &rigid_bodies->bodies[i]);
        rigid_bodies->velocities[i] = vec_entry_mult(rigid_bodies->velocities[i], v);
        rigid_bodies->movements[i] = vec_entry_mult(rigid_bodies->movements[i], v);
        rigid_bodies_damper(rigid_bodies, i, vec_entry_mult(v, vec(-16.0f, 0.0f)));
    }

    return 0;
}

int rigid_bodies_collide(RigidBodies *rigid_bodies,
                         const Platforms *platforms)
{
    if (rigid_bodies_collide_with_itself(rigid_bodies) < 0) {
        return -1;
    }

    if (rigid_bodies_collide_with_platforms(rigid_bodies, platforms) < 0) {
        return -1;
    }

    return 0;
}

int rigid_bodies_update(RigidBodies *rigid_bodies,
                        float delta_time)
{
    trace_assert(rigid_bodies);

    memset(rigid_bodies->grounded, 0,
           sizeof(bool) * rigid_bodies->count);

    for (size_t i = 0; i < rigid_bodies->count; ++i) {
        rigid_bodies->velocities[i] = vec_sum(
            rigid_bodies->velocities[i],
            vec_scala_mult(
                rigid_bodies->forces[i],
                delta_time));
    }

    for (size_t i = 0; i < rigid_bodies->count; ++i) {
        Vec position = vec(rigid_bodies->bodies[i].x,
                           rigid_bodies->bodies[i].y);

        position = vec_sum(
            position,
            vec_scala_mult(
                vec_sum(
                    rigid_bodies->velocities[i],
                    rigid_bodies->movements[i]),
                delta_time));

        rigid_bodies->bodies[i].x = position.x;
        rigid_bodies->bodies[i].y = position.y;
    }

    memset(rigid_bodies->forces, 0,
           sizeof(Vec) * rigid_bodies->count);

    return 0;
}

int rigid_bodies_render(RigidBodies *rigid_bodies,
                        Camera *camera)
{
    trace_assert(rigid_bodies);
    trace_assert(camera);

    char text_buffer[256];

    for (size_t i = 0; i < rigid_bodies->count; ++i) {
        if (camera_fill_rect(
                camera,
                rigid_bodies->bodies[i],
                rigid_bodies->colors[i]) < 0) {
            return -1;
        }

        snprintf(text_buffer, ID_SIZE, "%ld", i);

        if (camera_render_debug_text(
                camera,
                text_buffer,
                vec(rigid_bodies->bodies[i].x,
                    rigid_bodies->bodies[i].y)) < 0) {
            return -1;
        }

        snprintf(text_buffer, 256, "(%f, %f)",
                 rigid_bodies->bodies[i].x,
                 rigid_bodies->bodies[i].y);
        if (camera_render_debug_text(
                camera,
                text_buffer,
                vec(rigid_bodies->bodies[i].x,
                    // TODO: the size of the debug text is unknown in rigid_bodies_render()
                    rigid_bodies->bodies[i].y + 25.0f))) {
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
    rigid_bodies->bodies[id] = rect;
    rigid_bodies->colors[id] = color;

    return id;
}

Rect rigid_bodies_hitbox(const RigidBodies *rigid_bodies,
                         RigidBodyId id)
{
    trace_assert(rigid_bodies);
    trace_assert(id < rigid_bodies->count);

    return rigid_bodies->bodies[id];
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

void rigid_bodies_apply_omniforce(RigidBodies *rigid_bodies,
                                  Vec force)
{
    for (size_t i = 0; i < rigid_bodies->count; ++i) {
        rigid_bodies_apply_force(rigid_bodies, i, force);
    }
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

    rigid_bodies->bodies[id].x = position.x;
    rigid_bodies->bodies[id].y = position.y;
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
