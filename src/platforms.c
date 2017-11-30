#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "./platforms.h"

struct platforms_t {
    struct rect_t *rects;
    size_t rects_size;
};

struct platforms_t *create_platforms(const struct rect_t *rects, size_t rects_size)
{
    assert(rects);

    struct platforms_t *platforms = malloc(sizeof(struct platforms_t));

    if (platforms == NULL) {
        goto platforms_malloc_fail;
    }

    platforms->rects = malloc(sizeof(struct rect_t) * rects_size);

    if (platforms->rects == NULL) {
        goto platforms_rects_malloc_fail;
    }

    platforms->rects = memcpy(platforms->rects, rects, sizeof(struct rect_t) * rects_size);
    platforms->rects_size = rects_size;

    return platforms;

platforms_rects_malloc_fail:
    free(platforms);
platforms_malloc_fail:
    return NULL;
}

void destroy_platforms(struct platforms_t *platforms)
{
    assert(platforms);

    free(platforms->rects);
    free(platforms);
}

int render_platforms(const struct platforms_t *platforms,
                     SDL_Renderer *renderer)
{
    if (SDL_SetRenderDrawColor(renderer, 255, 96, 96, 255) < 0) {
        return -1;
    }

    SDL_Rect rect;

    for (size_t i = 0; i < platforms->rects_size; ++i) {
        rect.x = roundf(platforms->rects[i].x);
        rect.y = roundf(platforms->rects[i].y);
        rect.w = roundf(platforms->rects[i].w);
        rect.h = roundf(platforms->rects[i].h);

        if (SDL_RenderFillRect(renderer, &rect) < 0) {
            return -1;
        }
    }

    return 0;
}
