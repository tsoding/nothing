#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

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
