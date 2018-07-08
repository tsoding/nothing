#include <SDL2/SDL.h>
#include <assert.h>

#include "renderer.h"
#include "system/error.h"
#include "system/lt.h"

int draw_triangle(SDL_Renderer *render,
                  triangle_t t)
{
    assert(render);

    if (SDL_RenderDrawLine(render,
                           (int) roundf(t.p1.x),
                           (int) roundf(t.p1.y),
                           (int) roundf(t.p2.x),
                           (int) roundf(t.p2.y)) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    if (SDL_RenderDrawLine(render,
                           (int) roundf(t.p2.x),
                           (int) roundf(t.p2.y),
                           (int) roundf(t.p3.x),
                           (int) roundf(t.p3.y)) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    if (SDL_RenderDrawLine(render,
                           (int) roundf(t.p3.x),
                           (int) roundf(t.p3.y),
                           (int) roundf(t.p1.x),
                           (int) roundf(t.p1.y)) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    return 0;
}

static int fill_bottom_flat_triangle(SDL_Renderer *render,
                                     triangle_t t)
{
    assert(render);

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
                                  triangle_t t)
{
    assert(render);

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
                  triangle_t t)
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
        const point_t p4 = vec(t.p1.x + ((t.p2.y - t.p1.y) / (t.p3.y - t.p1.y)) * (t.p3.x - t.p1.x), t.p2.y);

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

int render_text_shaded(SDL_Renderer *renderer,
                       TTF_Font *font,
                       vec_t position,
                       color_t color,
                       color_t bg,
                       const char *text)
{
    assert(renderer);
    assert(font);
    assert(text);

    lt_t * const lt = create_lt();
    if (lt == NULL) {
        return -1;
    }

    SDL_Surface * const surface = PUSH_LT(
        lt,
        TTF_RenderText_Shaded(font, text, color_for_sdl(color), color_for_sdl(bg)),
        SDL_FreeSurface);
    if (surface == NULL) {
        throw_error(ERROR_TYPE_SDL2_TTF);
        RETURN_LT(lt, -1);
    }

    SDL_Texture * const texture = PUSH_LT(
        lt,
        SDL_CreateTextureFromSurface(renderer, surface),
        SDL_DestroyTexture);
    if (texture == NULL) {
        throw_error(ERROR_TYPE_SDL2);
        RETURN_LT(lt, -1);
    }

    int texW = 0;
    int texH = 0;

    if (SDL_QueryTexture(texture, NULL, NULL, &texW, &texH) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        RETURN_LT(lt, -1);
    }

    const SDL_Rect dstrect = { (int) position.x, (int) position.y, texW, texH };

    if (SDL_RenderCopy(renderer, texture, NULL, &dstrect) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        RETURN_LT(lt, -1);
    }

    RETURN_LT(lt, 0);
}
