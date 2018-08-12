#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>

#include "color.h"
#include "game/level/lava/wavy_rect.h"
#include "lava.h"
#include "math/rect.h"
#include "system/error.h"
#include "system/lt.h"

struct lava_t {
    lt_t *lt;
    size_t rects_count;
    wavy_rect_t **rects;
};

lava_t *create_lava_from_stream(FILE *stream)
{
    assert(stream);

    lt_t *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    lava_t *lava = PUSH_LT(lt, malloc(sizeof(lava_t)), free);
    if (lava == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    if (fscanf(stream, "%lu", &lava->rects_count) < 0) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    lava->rects = PUSH_LT(lt, malloc(sizeof(wavy_rect_t*) * lava->rects_count), free);
    if (lava->rects == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    for (size_t i = 0; i < lava->rects_count; ++i) {
        lava->rects[i] = PUSH_LT(lt, create_wavy_rect_from_stream(stream), destroy_wavy_rect);
        if (lava->rects[i] == NULL) {
            RETURN_LT(lt, NULL);
        }
    }

    lava->lt = lt;

    return lava;
}

void destroy_lava(lava_t *lava)
{
    assert(lava);
    RETURN_LT0(lava->lt);
}

int lava_render(const lava_t   *lava,
                camera_t *camera)
{
    assert(lava);
    assert(camera);

    for (size_t i = 0; i < lava->rects_count; ++i) {
        if (wavy_rect_render(lava->rects[i], camera) < 0) {
            return -1;
        }
    }

    return 0;
}

int lava_update(lava_t *lava, float delta_time)
{
    assert(lava);

    for (size_t i = 0; i < lava->rects_count; ++i) {
        if (wavy_rect_update(lava->rects[i], delta_time) < 0) {
            return -1;
        }
    }

    return 0;
}

bool lava_overlaps_rect(const lava_t *lava,
                        rect_t rect)
{
    assert(lava);

    for (size_t i = 0; i < lava->rects_count; ++i) {
        if (wavy_rect_overlaps(lava->rects[i], rect)) {
            return true;
        }
    }

    return 0;
}

void lava_float_solid(const lava_t *lava, solid_ref_t solid)
{
    assert(lava);
    (void) solid;
}
