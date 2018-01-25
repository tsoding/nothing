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

static void swap_points(point_t *p1, point_t *p2)
{
    point_t t = *p1;
    *p1 = *p2;
    *p2 = t;
}

static int fill_bottom_flat_triangle(SDL_Renderer *render,
                                     point_t p1,
                                     point_t p2,
                                     point_t p3)
{
    assert(render);

    const float invslope1 = (p2.x - p1.x) / (p2.y - p1.y);
    const float invslope2 = (p3.x - p1.x) / (p3.y - p1.y);

    float curx1 = p1.x;
    float curx2 = p1.x;

    for (int scanlineY = (int) roundf(p1.y);
         scanlineY <= (int) roundf(p2.y);
         scanlineY++) {
        if (SDL_RenderDrawLine(render,
                               (int) roundf(curx1),
                               scanlineY,
                               (int) roundf(curx2),
                               scanlineY) < 0) {
            return -1;
        }
        curx1 += invslope1;
        curx2 += invslope2;
    }

    return 0;
}

static int fill_top_flat_triangle(SDL_Renderer *render,
                                   point_t p1,
                                   point_t p2,
                                   point_t p3)
{
    assert(render);

    const float invslope1 = (p3.x - p1.x) / (p3.y - p1.y);
    const float invslope2 = (p3.x - p2.x) / (p3.y - p2.y);

    float curx1 = p3.x;
    float curx2 = p3.x;

    for (int scanlineY = (int) roundf(p3.y);
         scanlineY > (int) roundf(p1.y);
         --scanlineY) {
        if (SDL_RenderDrawLine(render,
                               (int) roundf(curx1),
                               scanlineY,
                               (int) roundf(curx2),
                               scanlineY) < 0) {
            return -1;
        }

        curx1 -= invslope1;
        curx2 -= invslope2;
    }

    return 0;
}

int fill_triangle(SDL_Renderer *render,
                  point_t p1,
                  point_t p2,
                  point_t p3)
{
    if (p1.y > p2.y) { swap_points(&p1, &p2); }
    if (p2.y > p3.y) { swap_points(&p2, &p3); }
    if (p1.y > p2.y) { swap_points(&p1, &p2); }

    if (p2.y == p3.y) {
        if (fill_bottom_flat_triangle(render, p1, p2, p3) < 0) {
            return -1;
        }
    } else if (p1.y == p2.y) {
        if (fill_top_flat_triangle(render, p1, p2, p3) < 0) {
            return -1;
        }
    } else {
        const point_t p4 = vec(p1.x + ((p2.y - p1.y) / (p3.y - p1.y)) * (p3.x - p1.x), p2.y);

        if (fill_bottom_flat_triangle(render, p1, p2, p4) < 0) {
            return -1;
        }

        if (fill_top_flat_triangle(render, p2, p4, p3) < 0) {
            return -1;
        }
    }

    return 0;
}
