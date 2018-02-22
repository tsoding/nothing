#include <SDL2/SDL.h>
#include <stdio.h>
#include <assert.h>

#include "./lt.h"
#include "./lava.h"
#include "./error.h"
#include "./color.h"

struct lava_t {
    lt_t *lt;
    size_t rects_count;
    rect_t *rects;
    color_t *colors;
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

    lava->rects = PUSH_LT(lt, malloc(sizeof(rect_t) * lava->rects_count), free);
    if (lava->rects == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    lava->colors = PUSH_LT(lt, malloc(sizeof(color_t) * lava->rects_count), free);
    if (lava->colors == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    char color[7];
    for (size_t i = 0; i < lava->rects_count; ++i) {
        if (fscanf(stream, "%f%f%f%f%6s\n",
                   &lava->rects[i].x, &lava->rects[i].y,
                   &lava->rects[i].w, &lava->rects[i].h,
                   color) < 0) {
            throw_error(ERROR_TYPE_LIBC);
            RETURN_LT(lt, NULL);
        }

        lava->colors[i] = color_from_hexstr(color);
    }

    lava->lt = lt;

    return lava;
}

void destroy_lava(lava_t *lava)
{
    assert(lava);
    RETURN_LT0(lava->lt);
}

/* TODO: Lava rendering is too boring */
int lava_render(const lava_t   *lava,
                SDL_Renderer   *renderer,
                const camera_t *camera)
{
    assert(lava);
    assert(renderer);
    assert(camera);

    for (size_t i = 0; i < lava->rects_count; ++i) {
        if (camera_fill_rect(camera,
                             renderer,
                             lava->rects[i],
                             lava->colors[i]) < 0) {
            return -1;
        }
    }

    return 0;
}

int lava_update(lava_t *lava, Uint32 delta_time)
{
    assert(lava);
    assert(delta_time);

    return 0;
}

int lava_overlaps_rect(const lava_t *lava,
                      rect_t rect)
{
    assert(lava);

    for (size_t i = 0; i < lava->rects_count; ++i) {
        if (rects_overlap(rect, lava->rects[i])) {
            return 1;
        }
    }

    return 0;
}
