#include <SDL.h>
#include "system/stacktrace.h"
#include <stdio.h>
#include <string.h>

#include "math/rect.h"
#include "sdl/renderer.h"
#include "sprite_font.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/log.h"

#define FONT_ROW_SIZE 18

struct Sprite_font
{
    SDL_Texture *texture;
};

static inline
void *scp(void *ptr)
{
    if (ptr == NULL) {
        log_fail("SDL error: %s\n", SDL_GetError());
        trace_assert(0 && "SDL error");
    }

    return ptr;
}

static inline
int scc(int code)
{
    if (code < 0) {
        log_fail("SDL error: %s\n", SDL_GetError());
        trace_assert(0 && "SDL error");
    }

    return code;
}

SDL_Texture *load_bmp_font_texture(SDL_Renderer *renderer,
                                   const char *bmp_file_path)
{
    trace_assert(renderer);
    trace_assert(bmp_file_path);

    SDL_Surface *surface = scp(SDL_LoadBMP(bmp_file_path));
    scc(SDL_SetColorKey(
            surface,
            SDL_TRUE,
            SDL_MapRGB(surface->format,
                       0, 0, 0)));

    SDL_Texture *result =
        scp(SDL_CreateTextureFromSurface(renderer, surface));

    SDL_FreeSurface(surface);

    return result;
}

static SDL_Rect sprite_font_char_rect(const Sprite_font *sprite_font, char x)
{
    trace_assert(sprite_font);

    if (32 <= x && x <= 126) {
        const SDL_Rect rect = {
            .x = ((x - 32) % FONT_ROW_SIZE) * FONT_CHAR_WIDTH,
            .y = ((x - 32) / FONT_ROW_SIZE) * FONT_CHAR_HEIGHT,
            .w = FONT_CHAR_WIDTH,
            .h = FONT_CHAR_HEIGHT
        };
        return rect;
    } else {
        return sprite_font_char_rect(sprite_font, '?');
    }
}

void sprite_font_render_text(const Sprite_font *sprite_font,
                             SDL_Renderer *renderer,
                             Vec2f position,
                             Vec2f size,
                             Color color,
                             const char *text)
{
    trace_assert(sprite_font);
    trace_assert(renderer);
    trace_assert(text);

    const SDL_Color sdl_color = color_for_sdl(color);

    scc(SDL_SetTextureColorMod(sprite_font->texture,
                               sdl_color.r,
                               sdl_color.g,
                               sdl_color.b));
    scc(SDL_SetTextureAlphaMod(sprite_font->texture,
                               sdl_color.a));

    const size_t text_size = strlen(text);
    for (size_t i = 0, col = 0, row = 0; i < text_size; ++i) {
        if (text[i] == '\n'){
            col = 0;
            row++;
            continue;
        }
        const SDL_Rect char_rect = sprite_font_char_rect(sprite_font, text[i]);
        const SDL_Rect dest_rect = rect_for_sdl(
            rect(
                position.x + (float) FONT_CHAR_WIDTH * (float) col * size.x,
                position.y + (float) FONT_CHAR_HEIGHT * (float) row * size.y,
                (float) char_rect.w * size.x,
                (float) char_rect.h * size.y));
        scc(SDL_RenderCopy(renderer, sprite_font->texture, &char_rect, &dest_rect));
        col++;
    }
}
