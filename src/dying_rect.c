#include <assert.h>
#include <SDL2/SDL.h>

#include "./dying_rect.h"

dying_rect_t *create_dying_rect(rect_t rect,
                                color_t color,
                                Uint32 duration)
{
    (void) rect;
    (void) color;
    (void) duration;
    return NULL;
}

void destroy_dying_rect(dying_rect_t *dying_rect)
{
    assert(dying_rect);
}

int dying_rect_render(const dying_rect_t *dying_rect,
                      SDL_Renderer *renderer,
                      const camera_t *camera)
{
    assert(dying_rect);
    assert(renderer);
    assert(camera);
    return 0;
}

int dying_rect_update(dying_rect_t *dying_rect,
                      Uint32 delta_time)
{
    assert(dying_rect);
    assert(delta_time > 0);

    return 0;
}

int dying_rect_is_dead(const dying_rect_t *dying_rect)
{
    assert(dying_rect);
    return 0;
}
