#include <assert.h>
#include <SDL2/SDL.h>
#include "./glitchy_rect.h"
#include "./lt.h"
#include "./camera.h"
#include "./error.h"

struct glitchy_rect_t
{
    lt_t *lt;
    rect_t rect;
    color_t color;
};

glitchy_rect_t *create_glitchy_rect(rect_t rect, color_t color)
{
    lt_t *lt = create_lt();

    if (lt == NULL) {
        return NULL;
    }

    glitchy_rect_t * const glitchy_rect = PUSH_LT(
        lt,
        malloc(sizeof(glitchy_rect_t)),
        free);

    glitchy_rect->rect = rect;
    glitchy_rect->color = color;
    glitchy_rect->lt = lt;

    return glitchy_rect;
}

glitchy_rect_t *create_glitchy_rect_from_stream(FILE *stream)
{
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

    return create_glitchy_rect(rect, color);
}

void destroy_glitchy_rect(glitchy_rect_t *glitchy_rect)
{
    assert(glitchy_rect);
    RETURN_LT0(glitchy_rect->lt);
}

int glitchy_rect_render(const glitchy_rect_t *glitchy_rect,
                        SDL_Renderer *renderer,
                        const camera_t *camera)
{
    assert(glitchy_rect);
    assert(renderer);
    assert(camera);

    return camera_fill_rect(
        camera,
        renderer,
        glitchy_rect->rect,
        glitchy_rect->color);
}

int glitchy_rect_update(glitchy_rect_t *glitchy_rect,
                        Uint32 delta_time)
{
    assert(glitchy_rect);
    assert(delta_time > 0);

    return 0;
}

int glitchy_rect_overlaps(const glitchy_rect_t *glitchy_rect,
                          rect_t rect)
{
    return rects_overlap(glitchy_rect->rect, rect);
}
