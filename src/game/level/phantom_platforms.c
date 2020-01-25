#include "phantom_platforms.h"

Phantom_Platforms create_phantom_platforms(RectLayer *rect_layer)
{
    Phantom_Platforms pp;

    pp.size = rect_layer->rects.count;
    pp.rects = malloc(sizeof(pp.rects[0]) * pp.size);
    memcpy(pp.rects, rect_layer->rects.data, sizeof(pp.rects[0]) * pp.size);

    pp.colors = malloc(sizeof(pp.colors[0]) * pp.size);
    memcpy(pp.colors, rect_layer->colors.data, sizeof(pp.colors[0]) * pp.size);

    pp.hiding = calloc(1, sizeof(pp.hiding[0]) * pp.size);

    return pp;
}

void destroy_phantom_platforms(Phantom_Platforms pp)
{
    free(pp.rects);
    free(pp.colors);
    free(pp.hiding);
}

void phantom_platforms_render(const Phantom_Platforms *pp, const Camera *camera)
{
    trace_assert(pp);
    trace_assert(camera);

    for (size_t i = 0; i < pp->size; ++i) {
        camera_fill_rect(camera, pp->rects[i], pp->colors[i]);
    }
}

#define HIDING_SPEED 4.0f

// TODO(#1247): phantom_platforms_update is O(N) even when nothing is animated
void phantom_platforms_update(Phantom_Platforms *pp, float dt)
{
    trace_assert(pp);

    for (size_t i = 0; i < pp->size; ++i) {
        if (pp->hiding[i]) {
            if (pp->colors[i].a > 0.0f) {
                pp->colors[i].a =
                    fmaxf(0.0f, pp->colors[i].a - HIDING_SPEED * dt);
            } else {
                pp->hiding[i] = 0;
            }
        }
    }
}

// TODO(#1248): phantom_platforms_hide_at is O(N)
void phantom_platforms_hide_at(Phantom_Platforms *pp, Vec2f position)
{
    trace_assert(pp);

    for (size_t i = 0; i < pp->size; ++i) {
        if (rect_contains_point(pp->rects[i], position)) {
            pp->hiding[i] = 1;
        }
    }
}
