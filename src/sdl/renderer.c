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

/*
 * Return the pixel value at (x, y)
 * NOTE: The surface must be locked before calling this!
 */
Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return (Uint32) p[0] << 16 | (Uint32) p[1] << 8 | (Uint32) p[2];
        else
            return (Uint32) p[0] | (Uint32) p[1] << 8 | (Uint32) p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}

/*
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 */
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *(Uint32*) p = pixel;
        break;

    case 2:
        *(Uint16 *)p = (Uint16) pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}
