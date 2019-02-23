#include <stdlib.h>
#include <stdbool.h>

#include "game/camera.h"
#include "game/level/platforms.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/stacktrace.h"
#include "system/line_stream.h"
#include "system/str.h"
#include "system/log.h"

#include "./rigid_bodies.h"

#define RIGID_BODIES_MAX_ID_SIZE 36

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
    bool *deleted;
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

    rigid_bodies->deleted = PUSH_LT(lt, nth_calloc(capacity, sizeof(bool)), free);
    if (rigid_bodies->deleted == NULL) {
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

    // TODO(#709): rigid_bodies_collide_with_itself doesn't implement backpropagation
    //   https://github.com/tsoding/nothing/issues/704#issuecomment-466673040

    for (size_t i1 = 0; i1 < rigid_bodies->count - 1; ++i1) {
        if (rigid_bodies->deleted[i1]) {
            continue;
        }

        for (size_t i2 = i1 + 1; i2 < rigid_bodies->count; ++i2) {
            if (rigid_bodies->deleted[i2]) {
                continue;
            }

            if (!rects_overlap(rigid_bodies->bodies[i1], rigid_bodies->bodies[i2])) {
                continue;
            }

            Vec orient = rect_impulse(&rigid_bodies->bodies[i1], &rigid_bodies->bodies[i2]);

            if (orient.x > orient.y) {
                if (rigid_bodies->bodies[i1].y < rigid_bodies->bodies[i2].y) {
                    rigid_bodies->grounded[i1] = true;
                } else {
                    rigid_bodies->grounded[i2] = true;
                }
            }

            rigid_bodies_apply_force(
                rigid_bodies, i1, vec_sum(rigid_bodies->velocities[i2], rigid_bodies->movements[i2]));
            rigid_bodies_apply_force(
                rigid_bodies, i2, vec_sum(rigid_bodies->velocities[i1], rigid_bodies->movements[i1]));

            rigid_bodies->velocities[i1] = vec(rigid_bodies->velocities[i1].x * orient.x, rigid_bodies->velocities[i1].y * orient.y);
            rigid_bodies->velocities[i2] = vec(rigid_bodies->velocities[i2].x * orient.x, rigid_bodies->velocities[i2].y * orient.y);
            rigid_bodies->movements[i1] = vec(rigid_bodies->movements[i1].x * orient.x, rigid_bodies->movements[i1].y * orient.y);
            rigid_bodies->movements[i2] = vec(rigid_bodies->movements[i2].x * orient.x, rigid_bodies->movements[i2].y * orient.y);
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
        if (rigid_bodies->deleted[i]) {
            continue;
        }

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
    // TODO(#683): RigidBodies should collide only the bodies that were updated on after a previous collision
    memset(rigid_bodies->grounded, 0, sizeof(bool) * rigid_bodies->count);

    if (rigid_bodies_collide_with_itself(rigid_bodies) < 0) {
        return -1;
    }

    if (rigid_bodies_collide_with_platforms(rigid_bodies, platforms) < 0) {
        return -1;
    }

    return 0;
}

int rigid_bodies_update(RigidBodies *rigid_bodies,
                        RigidBodyId id,
                        float delta_time)
{
    trace_assert(rigid_bodies);

    rigid_bodies->velocities[id] = vec_sum(
            rigid_bodies->velocities[id],
            vec_scala_mult(
                rigid_bodies->forces[id],
                delta_time));

    Vec position = vec(rigid_bodies->bodies[id].x,
                       rigid_bodies->bodies[id].y);

    position = vec_sum(
        position,
        vec_scala_mult(
            vec_sum(
                rigid_bodies->velocities[id],
                rigid_bodies->movements[id]),
            delta_time));

    rigid_bodies->bodies[id].x = position.x;
    rigid_bodies->bodies[id].y = position.y;

    rigid_bodies->forces[id] = vec(0.0f, 0.0f);

    return 0;
}

int rigid_bodies_render(RigidBodies *rigid_bodies,
                        RigidBodyId id,
                        Camera *camera)
{
    trace_assert(rigid_bodies);
    trace_assert(camera);

    char text_buffer[256];

    if (camera_fill_rect(
            camera,
            rigid_bodies->bodies[id],
            rigid_bodies->colors[id]) < 0) {
        return -1;
    }

    snprintf(text_buffer, 256, "id: %ld", id);

    if (camera_render_debug_text(
            camera,
            text_buffer,
            vec(rigid_bodies->bodies[id].x,
                rigid_bodies->bodies[id].y)) < 0) {
        return -1;
    }

    snprintf(text_buffer, 256, "p:(%.2f, %.2f)",
             rigid_bodies->bodies[id].x,
             rigid_bodies->bodies[id].y);
    if (camera_render_debug_text(
            camera,
            text_buffer,
            vec(rigid_bodies->bodies[id].x,
                rigid_bodies->bodies[id].y + FONT_CHAR_HEIGHT * 2.0f))) {
        return -1;
    }

    snprintf(text_buffer, 256, "v:(%.2f, %.2f)",
             rigid_bodies->velocities[id].x,
             rigid_bodies->velocities[id].y);
    if (camera_render_debug_text(
            camera,
            text_buffer,
            vec(rigid_bodies->bodies[id].x,
                rigid_bodies->bodies[id].y + FONT_CHAR_HEIGHT * 4.0f))) {
        return -1;
    }

    snprintf(text_buffer, 256, "m:(%.2f, %.2f)",
             rigid_bodies->movements[id].x,
             rigid_bodies->movements[id].y);
    if (camera_render_debug_text(
            camera,
            text_buffer,
            vec(rigid_bodies->bodies[id].x,
                rigid_bodies->bodies[id].y + FONT_CHAR_HEIGHT * 6.0f))) {
        return -1;
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

void rigid_bodies_remove(RigidBodies *rigid_bodies,
                         RigidBodyId id)
{
    trace_assert(rigid_bodies);
    trace_assert(id < rigid_bodies->capacity);

    rigid_bodies->deleted[id] = true;
}

RigidBodyId rigid_bodies_add_from_line_stream(RigidBodies *rigid_bodies,
                                              LineStream *line_stream)
{
    trace_assert(rigid_bodies);
    trace_assert(line_stream);

    char color[7];
    Rect rect;
    // TODO(#686): id should be part of boxes
    char id[RIGID_BODIES_MAX_ID_SIZE];

    if (sscanf(line_stream_next(line_stream),
               "%" STRINGIFY(RIGID_BODIES_MAX_ID_SIZE) "s%f%f%f%f%6s\n",
               id,
               &rect.x, &rect.y,
               &rect.w, &rect.h,
               color) < 0) {
        log_fail("Could not read rigid rect\n");
        // TODO(#687): rigid_bodies_add_from_line_stream cannot indicate an error properly
        return 0;
    }

    return rigid_bodies_add(rigid_bodies, rect, hexstr(color));
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
