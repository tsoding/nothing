#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <SDL2/SDL.h>

#include "wavy_rect.h"

wavy_rect_t *create_wavy_rect(rect_t rect, color_t color)
{
    (void) rect;
    (void) color;
    return NULL;
}

wavy_rect_t *create_wavy_rect_from_stream(FILE *stream)
{
    assert(stream);
    return NULL;
}

void destroy_wavy_rect(wavy_rect_t *wavy_rect)
{
    assert(wavy_rect);
}

int wavy_rect_render(const wavy_rect_t *wavy_rect,
                     SDL_Renderer *renderer,
                     const camera_t *camera)
{
    assert(wavy_rect);
    assert(renderer);
    assert(camera);
    return 0;
}
int wavy_rect_update(wavy_rect_t *wavy_rect,
                     Uint32 delta_time)
{
    assert(wavy_rect);
    (void) delta_time;
    return 0;
}

int wavy_rect_overlaps(const wavy_rect_t *wavy_rect,
                       rect_t rect)
{
    assert(wavy_rect);
    (void) rect;
    return 0;
}
