#include <assert.h>
#include <SDL2/SDL.h>
#include "./glitchy_rect.h"

glitchy_rect_t *create_glitchy_rect(rect_t rect, color_t color)
{
    (void) rect;
    (void) color;
    return NULL;
}

void destroy_glitchy_rect(glitchy_rect_t *glitchy_rect)
{
    assert(glitchy_rect);
}

int glitchy_rect_render(const glitchy_rect_t *glitchy_rect,
                        SDL_Renderer *renderer,
                        const camera_t *camera)
{
    assert(glitchy_rect);
    assert(renderer);
    assert(camera);

    return 0;
}

int glitchy_rect_update(glitchy_rect_t *glitchy_rect,
                        Uint32 delta_time)
{
    assert(glitchy_rect);
    assert(delta_time > 0);

    return 0;
}
