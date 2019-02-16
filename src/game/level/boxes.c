#include "system/stacktrace.h"

#include "game/level/boxes.h"
#include "game/level/player.h"
#include "game/level/player/rigid_rect.h"
#include "system/line_stream.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/log.h"
#include "ebisp/interpreter.h"
#include "ebisp/builtins.h"
#include "broadcast.h"
#include "game/level/rigid_bodies.h"

/* TODO(#631): Boxes entity should be implemented in terms of RigidBodies instead of Rigid_rect */

#define BOXES_CAPACITY 100

struct Boxes
{
    Lt *lt;
    RigidBodies *rigid_bodies;
    RigidBodyId *body_ids;
    size_t count;
};

Boxes *create_boxes_from_line_stream(LineStream *line_stream, RigidBodies *rigid_bodies)
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
    boxes->lt = lt;

    boxes->rigid_bodies = rigid_bodies;

    if (sscanf(
            line_stream_next(line_stream),
            "%lu",
            &boxes->count) == EOF) {
        log_fail("Could not read amount of boxes\n");
        RETURN_LT(lt, NULL);
    }

    boxes->body_ids = PUSH_LT(lt, nth_alloc(sizeof(RigidBodyId) * boxes->count), free);
    if (boxes->body_ids == NULL) {
        RETURN_LT(lt, NULL);
    }

    for (size_t i = 0; i < boxes->count; ++i) {
        boxes->body_ids[i] = rigid_bodies_add_from_line_stream(boxes->rigid_bodies, line_stream);
    }

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
        if (rigid_bodies_render(boxes->rigid_bodies, boxes->body_ids[i], camera) < 0) {
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
        if (rigid_bodies_update(boxes->rigid_bodies, boxes->body_ids[i], delta_time) < 0) {
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
        lava_float_rigid_body(lava, boxes->rigid_bodies, boxes->body_ids[i]);
    }
}

static
int boxes_add_box(Boxes *boxes, Rect rect, Color color)
{
    trace_assert(boxes);
    trace_assert(boxes->count < BOXES_CAPACITY);

    boxes->body_ids[boxes->count++] = rigid_bodies_add(boxes->rigid_bodies, rect, color);

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

    if (symbol_p(target)) {
        const char *action = target.atom->str;

        if (strcmp(action, "new") == 0) {
            const char *color = NULL;
            long int x, y, w, h;
            res = match_list(gc, "dddds", rest, &x, &y, &w, &h, &color);
            if (res.is_error) {
                return res;
            }

            boxes_add_box(boxes, rect((float) x, (float) y, (float) w, (float) h), hexstr(color));

            return eval_success(NIL(gc));
        }

        return unknown_target(gc, "box", action);
    }

    return wrong_argument_type(gc, "string-or-symbol-p", target);
}
