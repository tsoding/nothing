#include <assert.h>
#include <SDL2/SDL.h>
#include "./renderer.h"
#include "./error.h"

int draw_triangle(SDL_Renderer *render,
                  point_t p1,
                  point_t p2,
                  point_t p3)
{
    assert(render);

    if (SDL_RenderDrawLine(render,
                           (int) roundf(p1.x),
                           (int) roundf(p1.y),
                           (int) roundf(p2.x),
                           (int) roundf(p2.y)) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    if (SDL_RenderDrawLine(render,
                           (int) roundf(p2.x),
                           (int) roundf(p2.y),
                           (int) roundf(p3.x),
                           (int) roundf(p3.y)) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    if (SDL_RenderDrawLine(render,
                           (int) roundf(p3.x),
                           (int) roundf(p3.y),
                           (int) roundf(p1.x),
                           (int) roundf(p1.y)) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    return 0;
}

int fill_triangle(SDL_Renderer *render,
                  point_t p1,
                  point_t p2,
                  point_t p3)
{
    /* TODO(#79): implement fill_triangle */
    return draw_triangle(render, p1, p2, p3);
}
