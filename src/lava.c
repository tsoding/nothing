#include <SDL2/SDL.h>
#include <stdio.h>
#include <assert.h>

#include "./lt.h"
#include "./lava.h"

struct lava_t {
    lt_t *lt;
};

lava_t *create_lava_from_stream(FILE *stream)
{
    assert(stream);
    return NULL;
}

void destroy_lava(lava_t *lava)
{
    assert(lava);
}

int lava_render(const lava_t   *lava,
                SDL_Renderer   *renderer,
                const camera_t *camera)
{
    assert(lava);
    assert(renderer);
    assert(camera);

    return 0;
}

int lava_update(lava_t *lava, Uint32 delta_time)
{
    assert(lava);
    assert(delta_time);

    return 0;
}

int lava_contans_rect(const lava_t *lava,
                      rect_t rect)
{
    assert(lava);
    (void) rect;

    return 0;
}
