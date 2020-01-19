#include <SDL.h>
#include "system/stacktrace.h"
#include <stdio.h>

#include "color.h"
#include "game/level/lava/wavy_rect.h"
#include "lava.h"
#include "math/rect.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/log.h"
#include "game/level/level_editor/rect_layer.h"

#define LAVA_BOINGNESS 2500.0f

struct Lava {
    Lt *lt;
    size_t rects_count;
    Wavy_rect **rects;
};

Lava *create_lava_from_rect_layer(const RectLayer *rect_layer)
{
    Lt *lt = create_lt();

    Lava *lava = PUSH_LT(lt, nth_calloc(1, sizeof(Lava)), free);
    if (lava == NULL) {
        RETURN_LT(lt, NULL);
    }
    lava->lt = lt;

    lava->rects_count = rect_layer_count(rect_layer);
    lava->rects = PUSH_LT(lt, nth_calloc(lava->rects_count, sizeof(Wavy_rect*)), free);
    if (lava->rects == NULL) {
        RETURN_LT(lt, NULL);
    }

    const Rect *rects = rect_layer_rects(rect_layer);
    const Color *colors = rect_layer_colors(rect_layer);
    for (size_t i = 0; i < lava->rects_count; ++i) {
        lava->rects[i] = PUSH_LT(lt, create_wavy_rect(rects[i], colors[i]), destroy_wavy_rect);
        if (lava->rects[i] == NULL) {
            RETURN_LT(lt, NULL);
        }
    }

    return lava;
}

void destroy_lava(Lava *lava)
{
    trace_assert(lava);
    RETURN_LT0(lava->lt);
}

/* TODO(#449): lava does not render its id in debug mode */
int lava_render(const Lava *lava,
                const Camera *camera)
{
    trace_assert(lava);
    trace_assert(camera);

    for (size_t i = 0; i < lava->rects_count; ++i) {
        if (wavy_rect_render(lava->rects[i], camera) < 0) {
            return -1;
        }
    }

    return 0;
}

int lava_update(Lava *lava, float delta_time)
{
    trace_assert(lava);

    for (size_t i = 0; i < lava->rects_count; ++i) {
        if (wavy_rect_update(lava->rects[i], delta_time) < 0) {
            return -1;
        }
    }

    return 0;
}

bool lava_overlaps_rect(const Lava *lava,
                        Rect rect)
{
    trace_assert(lava);

    for (size_t i = 0; i < lava->rects_count; ++i) {
        if (rects_overlap(wavy_rect_hitbox(lava->rects[i]), rect)) {
            return true;
        }
    }

    return 0;
}

void lava_float_rigid_body(Lava *lava, RigidBodies *rigid_bodies, RigidBodyId id)
{
    trace_assert(lava);

    const Rect object_hitbox = rigid_bodies_hitbox(rigid_bodies, id);
    for (size_t i = 0; i < lava->rects_count; ++i) {
        const Rect lava_hitbox = wavy_rect_hitbox(lava->rects[i]);
        if (rects_overlap(object_hitbox, lava_hitbox)) {
            const Rect overlap_area = rects_overlap_area(object_hitbox, lava_hitbox);
            const float k = overlap_area.w * overlap_area.h / (object_hitbox.w * object_hitbox.h);
            rigid_bodies_apply_force(
                rigid_bodies,
                id,
                vec(0.0f, -k * LAVA_BOINGNESS));
            rigid_bodies_damper(rigid_bodies, id, vec(0.0f, -0.9f));
        }
    }
}
