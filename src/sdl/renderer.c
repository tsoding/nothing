#include <SDL.h>
#include "system/stacktrace.h"

#include "renderer.h"
#include "system/lt.h"
#include "system/log.h"

int draw_triangle(SDL_Renderer *render,
                  Triangle t)
{
    trace_assert(render);

    if (SDL_RenderDrawLine(render,
                           (int) roundf(t.p1.x),
                           (int) roundf(t.p1.y),
                           (int) roundf(t.p2.x),
                           (int) roundf(t.p2.y)) < 0) {
        log_fail("SDL_RenderDrawLine: %s\n", SDL_GetError());
        return -1;
    }

    if (SDL_RenderDrawLine(render,
                           (int) roundf(t.p2.x),
                           (int) roundf(t.p2.y),
                           (int) roundf(t.p3.x),
                           (int) roundf(t.p3.y)) < 0) {
        log_fail("SDL_RenderDrawLine: %s\n", SDL_GetError());
        return -1;
    }

    if (SDL_RenderDrawLine(render,
                           (int) roundf(t.p3.x),
                           (int) roundf(t.p3.y),
                           (int) roundf(t.p1.x),
                           (int) roundf(t.p1.y)) < 0) {
        log_fail("SDL_RenderDrawLine: %s\n", SDL_GetError());
        return -1;
    }

    return 0;
}

static int fill_bottom_flat_triangle(SDL_Renderer *render,
                                     Triangle t)
{
    trace_assert(render);

    const float invslope1 = (t.p2.x - t.p1.x) / (t.p2.y - t.p1.y);
    const float invslope2 = (t.p3.x - t.p1.x) / (t.p3.y - t.p1.y);

    const int y0 = (int) roundf(t.p1.y);
    const int y1 = (int) roundf(t.p2.y);

    float curx1 = t.p1.x;
    float curx2 = t.p1.x;

    for (int scanline = y0; scanline < y1; scanline++) {
        if (SDL_RenderDrawLine(render,
                               (int) roundf(curx1),
                               scanline,
                               (int) roundf(curx2),
                               scanline) < 0) {
            return -1;
        }
        curx1 += invslope1;
        curx2 += invslope2;
    }

    return 0;
}

static int fill_top_flat_triangle(SDL_Renderer *render,
                                  Triangle t)
{
    trace_assert(render);

    const float invslope1 = (t.p3.x - t.p1.x) / (t.p3.y - t.p1.y);
    const float invslope2 = (t.p3.x - t.p2.x) / (t.p3.y - t.p2.y);

    const int y0 = (int) roundf(t.p3.y);
    const int y1 = (int) roundf(t.p1.y);

    float curx1 = t.p3.x;
    float curx2 = t.p3.x;

    for (int scanline = y0; scanline > y1; --scanline) {
        if (SDL_RenderDrawLine(render,
                               (int) roundf(curx1),
                               scanline,
                               (int) roundf(curx2),
                               scanline) < 0) {
            return -1;
        }

        curx1 -= invslope1;
        curx2 -= invslope2;
    }

    return 0;
}

int fill_triangle(SDL_Renderer *render,
                  Triangle t)
{
    t = triangle_sorted_by_y(t);

    if (fabs(t.p2.y - t.p3.y) < 1e-6) {
        if (fill_bottom_flat_triangle(render, t) < 0) {
            return -1;
        }
    } else if (fabs(t.p1.y - t.p2.y) < 1e-6) {
        if (fill_top_flat_triangle(render, t) < 0) {
            return -1;
        }
    } else {
        const Vec2f p4 = vec(t.p1.x + ((t.p2.y - t.p1.y) / (t.p3.y - t.p1.y)) * (t.p3.x - t.p1.x), t.p2.y);

        if (fill_bottom_flat_triangle(render, triangle(t.p1, t.p2, p4)) < 0) {
            return -1;
        }

        if (fill_top_flat_triangle(render, triangle(t.p2, p4, t.p3)) < 0) {
            return -1;
        }

        if (SDL_RenderDrawLine(render,
                               (int) roundf(t.p2.x),
                               (int) roundf(t.p2.y),
                               (int) roundf(p4.x),
                               (int) roundf(p4.y)) < 0) {
            return -1;
        }
    }

    return 0;
}

int fill_rect(SDL_Renderer *render, Rect r, Color c)
{
    const SDL_Rect sdl_rect = rect_for_sdl(r);
    const SDL_Color sdl_color = color_for_sdl(c);

    if (SDL_SetRenderDrawColor(
            render,
            sdl_color.r, sdl_color.g,
            sdl_color.b, sdl_color.a) < 0) {
        log_fail("SDL_SetRenderDrawColor: %s\n", SDL_GetError());
        return -1;
    }

    if (SDL_RenderFillRect(render, &sdl_rect) < 0) {
        log_fail("SDL_RenderFillRect: %s\n");
        return -1;
    }

    return 0;
}
