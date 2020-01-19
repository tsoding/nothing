#include "system/stacktrace.h"

#include "dynarray.h"
#include "game/level/boxes.h"
#include "game/level/level_editor/rect_layer.h"
#include "game/level/player.h"
#include "game/level/rigid_bodies.h"
#include "math/rand.h"
#include "system/log.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/str.h"
#include "config.h"

struct Boxes
{
    Lt *lt;
    RigidBodies *rigid_bodies;
    Dynarray boxes_ids;
    Dynarray body_ids;
    Dynarray body_colors;
};

Boxes *create_boxes_from_rect_layer(const RectLayer *layer, RigidBodies *rigid_bodies)
{
    trace_assert(layer);
    trace_assert(rigid_bodies);

    Lt *lt = create_lt();

    Boxes *boxes = PUSH_LT(lt, nth_calloc(1, sizeof(Boxes)), free);
    if (boxes == NULL) {
        RETURN_LT(lt, NULL);
    }
    boxes->lt = lt;

    boxes->boxes_ids = create_dynarray_malloc(ENTITY_MAX_ID_SIZE);
    boxes->body_ids = create_dynarray_malloc(sizeof(RigidBodyId));
    boxes->body_colors = create_dynarray_malloc(sizeof(Color));

    boxes->rigid_bodies = rigid_bodies;

    const size_t count = rect_layer_count(layer);
    Rect const *rects = rect_layer_rects(layer);
    Color const *colors = rect_layer_colors(layer);
    const char *ids = rect_layer_ids(layer);

    for (size_t i = 0; i < count; ++i) {
        RigidBodyId body_id = rigid_bodies_add(rigid_bodies, rects[i]);
        dynarray_push(&boxes->body_ids, &body_id);
        dynarray_push(&boxes->body_colors, &colors[i]);
        dynarray_push(&boxes->boxes_ids, ids + i * ENTITY_MAX_ID_SIZE);
    }

    return boxes;
}

void destroy_boxes(Boxes *boxes)
{
    trace_assert(boxes);

    RigidBodyId *body_ids = (RigidBodyId *)boxes->body_ids.data;
    for (size_t i = 0; i < boxes->body_ids.count; ++i) {
        rigid_bodies_remove(boxes->rigid_bodies, body_ids[i]);
    }

    free(boxes->boxes_ids.data);
    free(boxes->body_ids.data);
    free(boxes->body_colors.data);

    RETURN_LT0(boxes->lt);
}

int boxes_render(Boxes *boxes, const Camera *camera)
{
    trace_assert(boxes);
    trace_assert(camera);

    const size_t count = boxes->body_ids.count;
    RigidBodyId *body_ids = (RigidBodyId *)boxes->body_ids.data;
    Color *body_colors = (Color *)boxes->body_colors.data;

    for (size_t i = 0; i < count; ++i) {
        if (rigid_bodies_render(
                boxes->rigid_bodies,
                body_ids[i],
                body_colors[i],
                camera) < 0) {
            return -1;
        }
    }

    return 0;
}

int boxes_update(Boxes *boxes,
                 float delta_time)
{
    trace_assert(boxes);

    const size_t count = boxes->body_ids.count;
    RigidBodyId *body_ids = (RigidBodyId *)boxes->body_ids.data;

    for (size_t i = 0; i < count; ++i) {
        if (rigid_bodies_update(boxes->rigid_bodies, body_ids[i], delta_time) < 0) {
            return -1;
        }
    }

    return 0;
}

void boxes_float_in_lava(Boxes *boxes, Lava *lava)
{
    trace_assert(boxes);
    trace_assert(lava);

    const size_t count = boxes->body_ids.count;
    RigidBodyId *body_ids = (RigidBodyId*)boxes->body_ids.data;

    for (size_t i = 0; i < count; ++i) {
        lava_float_rigid_body(lava, boxes->rigid_bodies, body_ids[i]);
    }
}

int boxes_add_box(Boxes *boxes, Rect rect, Color color)
{
    trace_assert(boxes);

    RigidBodyId body_id = rigid_bodies_add(boxes->rigid_bodies, rect);
    dynarray_push(&boxes->body_ids, &body_id);
    dynarray_push(&boxes->body_colors, &color);

    return 0;
}

int boxes_delete_at(Boxes *boxes, Vec2f position)
{
    trace_assert(boxes);

    const size_t count = boxes->body_ids.count;
    RigidBodyId *body_ids = (RigidBodyId*)boxes->body_ids.data;

    for (size_t i = 0; i < count; ++i) {
        const Rect hitbox = rigid_bodies_hitbox(
            boxes->rigid_bodies,
            body_ids[i]);
        if (rect_contains_point(hitbox, position)) {
            rigid_bodies_remove(boxes->rigid_bodies, body_ids[i]);
            dynarray_delete_at(&boxes->body_ids, i);
            dynarray_delete_at(&boxes->body_colors, i);
            return 0;
        }
    }

    return 0;
}
