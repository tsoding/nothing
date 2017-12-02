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
                     SDL_Renderer *renderer,
                     const camera_t *camera)
{
    if (SDL_SetRenderDrawColor(renderer, 255, 96, 96, 255) < 0) {
        return -1;
    }

    for (size_t i = 0; i < platforms->rects_size; ++i) {
        if (camera_fill_rect(camera, renderer, &platforms->rects[i]) < 0) {
            return -1;
        }
    }

    return 0;
}

int platforms_rect_object_collide(const struct platforms_t *platforms,
                                  const struct rect_t *object)
{
    assert(platforms);
    assert(object);

    for (size_t i = 0; i < platforms->rects_size; ++i) {
        if (rects_intersect(object, &platforms->rects[i])) {
            return 1;
        }
    }

    return 0;
}
