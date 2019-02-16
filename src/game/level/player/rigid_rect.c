#include <SDL2/SDL.h>
#include "system/stacktrace.h"
#include <stdio.h>
#include <string.h>

#include "color.h"
#include "game/level/boxes.h"
#include "rigid_rect.h"
#include "system/str.h"
#include "system/line_stream.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/log.h"
#include "ebisp/interpreter.h"
#include "broadcast.h"

#define RIGID_RECT_MAX_ID_SIZE 36

struct Rigid_rect {
    Lt *lt;
    char *id;
    Vec position;
    Vec velocity;
    Vec movement;
    Vec size;
    Color color;
    int touches_ground;
    Vec forces;
};

Rigid_rect *create_rigid_rect(Rect rect, Color color, const char *id)
{
    trace_assert(id);

    Lt *lt = create_lt();

    if (lt == NULL) {
        return NULL;
    }

    Rigid_rect *rigid_rect = PUSH_LT(lt, nth_alloc(sizeof(Rigid_rect)), free);
    if (rigid_rect == NULL) {
        RETURN_LT(lt, NULL);
    }
    rigid_rect->lt = lt;

    rigid_rect->id = PUSH_LT(lt, nth_alloc(sizeof(char) * RIGID_RECT_MAX_ID_SIZE), free);
    if (rigid_rect->id == NULL) {
        RETURN_LT(lt, NULL);
    }

    const size_t _len = strlen(id);
    const size_t len_id = (RIGID_RECT_MAX_ID_SIZE-1) < _len ? (RIGID_RECT_MAX_ID_SIZE-1) : _len;
    memcpy(rigid_rect->id, id, len_id);
    rigid_rect->id[len_id] = 0;

    rigid_rect->position = vec(rect.x, rect.y);
    rigid_rect->velocity = vec(0.0f, 0.0f);
    rigid_rect->movement = vec(0.0f, 0.0f);
    rigid_rect->size = vec(rect.w, rect.h);
    rigid_rect->color = color;
    rigid_rect->touches_ground = 0;
    rigid_rect->forces = vec(0.0f, 0.0f);

    return rigid_rect;
}

Rigid_rect *create_rigid_rect_from_line_stream(LineStream *line_stream)
{
    trace_assert(line_stream);

    char color[7];
    Rect rect;
    char id[RIGID_RECT_MAX_ID_SIZE];

    if (sscanf(line_stream_next(line_stream),
               "%" STRINGIFY(RIGID_RECT_MAX_ID_SIZE) "s%f%f%f%f%6s\n",
               id,
               &rect.x, &rect.y,
               &rect.w, &rect.h,
               color) < 0) {
        log_fail("Could not read rigid rect\n");
        return NULL;
    }

    return create_rigid_rect(rect, hexstr(color), id);
}

void destroy_rigid_rect(Rigid_rect *rigid_rect)
{
    RETURN_LT0(rigid_rect->lt);
}

void rigid_rect_touches_rect_sides(Rigid_rect *rigid_rect,
                                   Rect object,
                                   int sides[RECT_SIDE_N])
{
    rect_object_impact(object, rigid_rect_hitbox(rigid_rect), sides);
}

int rigid_rect_render(const Rigid_rect *rigid_rect,
                      Camera *camera)
{
    char text_buffer[256];

    if (camera_fill_rect(
            camera,
            rigid_rect_hitbox(rigid_rect),
            rigid_rect->color) < 0) {
        return -1;
    }

    if (camera_render_debug_text(
            camera,
            rigid_rect->id,
            rigid_rect->position) < 0) {
        return -1;
    }

    snprintf(text_buffer, 256, "(%f, %f)",
             rigid_rect->position.x,
             rigid_rect->position.y);

    if (camera_render_debug_text(
            camera,
            text_buffer,
            vec(rigid_rect->position.x,
                rigid_rect->position.y + FONT_CHAR_HEIGHT * 2.0f))) {
        return -1;
    }

    return 0;
}

int rigid_rect_update(Rigid_rect * rigid_rect,
                      float delta_time)
{
    trace_assert(rigid_rect);

    rigid_rect->touches_ground = 0;

    rigid_rect->velocity = vec_sum(
        rigid_rect->velocity,
        vec_scala_mult(
            rigid_rect->forces,
            delta_time));

    rigid_rect->position = vec_sum(
        rigid_rect->position,
        vec_scala_mult(
            vec_sum(
                rigid_rect->velocity,
                rigid_rect->movement),
            delta_time));

    rigid_rect->forces = vec(0.0f, 0.0f);

    return 0;
}

Rect rigid_rect_hitbox(const Rigid_rect *rigid_rect)
{
    return rect_from_vecs(
        rigid_rect->position,
        rigid_rect->size);
}

void rigid_rect_move(Rigid_rect *rigid_rect,
                           Vec movement)
{
    rigid_rect->movement = movement;
}

int rigid_rect_touches_ground(const Rigid_rect *rigid_rect)
{
    return rigid_rect->touches_ground;
}

void rigid_rect_apply_force(Rigid_rect * rigid_rect,
                            Vec force)
{
    rigid_rect->forces = vec_sum(rigid_rect->forces, force);
}

void rigid_rect_transform_velocity(Rigid_rect *rigid_rect,
                                   mat3x3 trans_mat)
{
    rigid_rect->velocity = point_mat3x3_product(rigid_rect->velocity,
                                                trans_mat);
}

void rigid_rect_teleport_to(Rigid_rect *rigid_rect,
                            Vec position)
{
    rigid_rect->position = position;
}

void rigid_rect_damper(Rigid_rect *rigid_rect, Vec v)
{
    rigid_rect_apply_force(
        rigid_rect,
        vec(rigid_rect->velocity.x * v.x, rigid_rect->velocity.y * v.y));
}

bool rigid_rect_has_id(Rigid_rect *rigid_rect,
                       const char *id)
{
    trace_assert(rigid_rect);
    trace_assert(id);

    return strcmp(rigid_rect->id, id) == 0;
}

struct EvalResult
rigid_rect_send(Rigid_rect *rigid_rect, Gc *gc, struct Scope *scope, struct Expr path)
{
    trace_assert(rigid_rect);
    trace_assert(gc);
    trace_assert(scope);

    const char *target = NULL;
    struct Expr rest = void_expr();
    struct EvalResult res = match_list(gc, "q*", path, &target, &rest);
    if (res.is_error) {
        return res;
    }

    if (strcmp(target, "apply-force") == 0) {
        struct Expr force = void_expr();
        res = match_list(gc, "e*", rest, &force, NULL);
        if (res.is_error) {
            return res;
        }

        long int force_x = 0L;
        long int force_y = 0L;

        res = match_list(gc, "dd", force, &force_x, &force_y);
        if (res.is_error) {
            return res;
        }

        rigid_rect_apply_force(
            rigid_rect,
            vec((float) force_x, (float) force_y));

        return eval_success(NIL(gc));
    }

    return unknown_target(gc, rigid_rect->id, target);
}
