#include <SDL2/SDL.h>
#include <stdio.h>
#include <assert.h>

#include "./lt.h"
#include "./lava.h"
#include "./error.h"
#include "./color.h"
#include "./glitchy_rect.h"

struct lava_t {
    lt_t *lt;
    size_t rects_count;
    glitchy_rect_t **rects;
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

    lava->rects = PUSH_LT(lt, malloc(sizeof(glitchy_rect_t*) * lava->rects_count), free);
    if (lava->rects == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    for (size_t i = 0; i < lava->rects_count; ++i) {
        lava->rects[i] = PUSH_LT(lt, create_glitchy_rect_from_stream(stream), destroy_glitchy_rect);
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

/* TODO(#119): Lava rendering is too boring */
int lava_render(const lava_t   *lava,
                SDL_Renderer   *renderer,
                const camera_t *camera)
{
    assert(lava);
    assert(renderer);
    assert(camera);

    for (size_t i = 0; i < lava->rects_count; ++i) {
        if (glitchy_rect_render(lava->rects[i], renderer, camera) < 0) {
            return -1;
        }
    }

    return 0;
}

int lava_update(lava_t *lava, Uint32 delta_time)
{
    assert(lava);
    assert(delta_time);

    for (size_t i = 0; i < lava->rects_count; ++i) {
        if (glitchy_rect_update(lava->rects[i], delta_time) < 0) {
            return -1;
        }
    }

    return 0;
}

int lava_overlaps_rect(const lava_t *lava,
                      rect_t rect)
{
    assert(lava);

    for (size_t i = 0; i < lava->rects_count; ++i) {
        if (glitchy_rect_overlaps(lava->rects[i], rect)) {
            return 1;
        }
    }

    return 0;
}
