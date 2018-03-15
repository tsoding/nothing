#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <SDL2/SDL.h>

#include "./wavy_rect.h"
#include "./lt.h"
#include "./error.h"

struct wavy_rect_t
{
    lt_t *lt;

    rect_t rect;
    color_t color;
};

wavy_rect_t *create_wavy_rect(rect_t rect, color_t color)
{
    lt_t *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    wavy_rect_t *wavy_rect = PUSH_LT(lt, malloc(sizeof(wavy_rect_t)), free);
    if (wavy_rect == NULL) {
        throw_error(ERROR_TYPE_SDL2);
        RETURN_LT(lt, NULL);
    }

    wavy_rect->rect = rect;
    wavy_rect->color = color;
    wavy_rect->lt = lt;

    return wavy_rect;
}

wavy_rect_t *create_wavy_rect_from_stream(FILE *stream)
{
    assert(stream);
    char color_name[7];
    rect_t rect;

    if (fscanf(stream, "%f%f%f%f%6s\n",
               &rect.x, &rect.y,
               &rect.w, &rect.h,
               color_name) < 0) {
        throw_error(ERROR_TYPE_LIBC);
        return NULL;
    }

    color_t color = color_from_hexstr(color_name);

    return create_wavy_rect(rect, color);
}

void destroy_wavy_rect(wavy_rect_t *wavy_rect)
{
    assert(wavy_rect);
    RETURN_LT0(wavy_rect->lt);
}

int wavy_rect_render(const wavy_rect_t *wavy_rect,
                     SDL_Renderer *renderer,
                     const camera_t *camera)
{
    assert(wavy_rect);
    assert(renderer);
    assert(camera);
    return camera_fill_rect(
        camera,
        renderer,
        wavy_rect->rect,
        wavy_rect->color);
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
