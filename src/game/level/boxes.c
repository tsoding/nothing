#include "system/stacktrace.h"

#include "game/level/boxes.h"
#include "game/level/physical_world.h"
#include "game/level/player.h"
#include "game/level/player/rigid_rect.h"
#include "system/line_stream.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/log.h"
#include "ebisp/interpreter.h"
#include "ebisp/builtins.h"
#include "broadcast.h"

struct Boxes
{
    Lt *lt;
    size_t count;
    Rigid_rect **bodies;
    size_t capacity;
};

Boxes *create_boxes_from_line_stream(LineStream *line_stream)
{
    trace_assert(line_stream);

    Lt *lt = create_lt();

    if (lt == NULL) {
        return NULL;
    }

    Boxes *boxes = PUSH_LT(lt, nth_alloc(sizeof(Boxes)), free);
    if (boxes == NULL) {
        RETURN_LT(lt, NULL);
    }

    if (sscanf(
            line_stream_next(line_stream),
            "%lu",
            &boxes->count) == EOF) {
        log_fail("Could not read amount of boxes\n");
        RETURN_LT(lt, NULL);
    }
    boxes->capacity = boxes->count;

    boxes->bodies = PUSH_LT(lt, nth_alloc(sizeof(Rigid_rect*) * boxes->count), free);
    if (boxes->bodies == NULL) {
        RETURN_LT(lt, NULL);
    }

    for (size_t i = 0; i < boxes->count; ++i) {
        boxes->bodies[i] = PUSH_LT(
            lt,
            create_rigid_rect_from_line_stream(line_stream),
            destroy_rigid_rect);
        if (boxes->bodies[i] == NULL) {
            RETURN_LT(lt, NULL);
        }
    }

    boxes->lt = lt;

    return boxes;
}

void destroy_boxes(Boxes *boxes)
{
    trace_assert(boxes);
    RETURN_LT0(boxes->lt);
}

int boxes_render(Boxes *boxes, Camera *camera)
{
    trace_assert(boxes);
    trace_assert(camera);

    for (size_t i = 0; i < boxes->count; ++i) {
        if (rigid_rect_render(boxes->bodies[i], camera) < 0) {
            return -1;
        }
    }

    return 0;
}

int boxes_update(Boxes *boxes,
                 float delta_time)
{
    trace_assert(boxes);
    trace_assert(delta_time);

    for (size_t i = 0; i < boxes->count; ++i) {
        if (rigid_rect_update(boxes->bodies[i], delta_time) < 0) {
            return -1;
        }
    }

    return 0;
}

int boxes_add_to_physical_world(const Boxes *boxes,
                                Physical_world *physical_world)
{
    trace_assert(boxes);
    trace_assert(physical_world);

    for (size_t i = 0; i < boxes->count; ++i) {
        if (physical_world_add_solid(
                physical_world,
                rigid_rect_as_solid(boxes->bodies[i])) < 0) {
            return -1;
        }
    }

    return 0;
}

void boxes_float_in_lava(Boxes *boxes, Lava *lava)
{
    trace_assert(boxes);
    trace_assert(lava);

    for (size_t i = 0; i < boxes->count; ++i) {
        lava_float_rigid_rect(lava, boxes->bodies[i]);
    }
}

Rigid_rect *boxes_rigid_rect(Boxes *boxes, const char *id)
{
    trace_assert(boxes);
    trace_assert(id);

    for (size_t i = 0; i < boxes->count; ++i) {
        if (rigid_rect_has_id(boxes->bodies[i], id)) {
            return boxes->bodies[i];
        }
    }

    return 0;
}

static
int boxes_add_box(Boxes *boxes, Rect rect, Color color, const char *id)
{
    trace_assert(boxes);
    trace_assert(id);

    /* TODO(#628): boxes_add_box is unable to register a new box in the physical world */

    if (boxes->count >= boxes->capacity) {
        const size_t new_capacity = boxes->capacity * 2;

        Rigid_rect **const new_bodies = nth_realloc(
            boxes->bodies,
            sizeof(Rigid_rect*) * new_capacity);

        if (new_bodies == NULL) {
            return -1;
        }

        boxes->capacity = new_capacity;
        boxes->bodies = REPLACE_LT(
            boxes->lt,
            boxes->bodies,
            new_bodies);
    }

    boxes->bodies[boxes->count++] = create_rigid_rect(rect, color, id);

    return 0;
}

struct EvalResult
boxes_send(Boxes *boxes, Gc *gc, struct Scope *scope, struct Expr path)
{
    trace_assert(boxes);
    trace_assert(gc);
    trace_assert(scope);

    struct Expr target = void_expr();
    struct Expr rest = void_expr();
    struct EvalResult res = match_list(gc, "e*", path, &target, &rest);
    if (res.is_error) {
        return res;
    }

    if (string_p(target)) {
        const char *box_id = target.atom->str;

        for (size_t i = 0; i < boxes->count; ++i) {
            if (rigid_rect_has_id(boxes->bodies[i], box_id)) {
                return rigid_rect_send(boxes->bodies[i], gc, scope, rest);
            }
        }

        return unknown_target(gc, "box", box_id);
    } else if (symbol_p(target)) {
        const char *action = target.atom->str;

        if (strcmp(action, "new") == 0) {
            long int x, y, w, h;
            res = match_list(gc, "dddd", rest, &x, &y, &w, &h);
            if (res.is_error) {
                return res;
            }

            /* TODO: the color and id of added box is hardcoded */
            boxes_add_box(boxes, rect((float) x, (float) y, (float) w, (float) h), hexstr("a02c2c"), "khooy");

            return eval_success(NIL(gc));
        }

        return unknown_target(gc, "box", action);
    }

    return wrong_argument_type(gc, "string-or-symbol-p", target);
}
