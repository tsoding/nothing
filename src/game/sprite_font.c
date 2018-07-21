#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "sdl/renderer.h"
#include "sprite_font.h"
#include "system/error.h"
#include "system/lt.h"

#define FONT_ROW_SIZE 18
#define CHAR_WIDTH 7
#define CHAR_HEIGHT 9

struct sprite_font_t
{
    lt_t *lt;
    SDL_Texture *texture;
};

sprite_font_t *create_sprite_font_from_file(const char *bmp_file_path,
                                            SDL_Renderer *renderer)
{
    assert(bmp_file_path);
    assert(renderer);

    lt_t * const lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    sprite_font_t * const sprite_font = PUSH_LT(lt, malloc(sizeof(sprite_font_t)), free);
    if (sprite_font == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    SDL_Surface * const surface = PUSH_LT(lt, SDL_LoadBMP(bmp_file_path), SDL_FreeSurface);
    if (surface == NULL) {
        throw_error(ERROR_TYPE_SDL2);
        RETURN_LT(lt, NULL);
    }

    if (SDL_SetColorKey(surface,
                        SDL_TRUE,
                        SDL_MapRGB(surface->format,
                                   0, 0, 0)) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        RETURN_LT(lt, NULL);
    }

    sprite_font->texture = PUSH_LT(
        lt,
        SDL_CreateTextureFromSurface(renderer, surface),
        SDL_DestroyTexture);
    if (sprite_font->texture == NULL) {
        throw_error(ERROR_TYPE_SDL2);
        RETURN_LT(lt, NULL);
    }

    SDL_FreeSurface(RELEASE_LT(lt, surface));

    sprite_font->lt = lt;

    return sprite_font;
}

void destroy_sprite_font(sprite_font_t *sprite_font)
{
    assert(sprite_font);
    RETURN_LT0(sprite_font->lt);
}

static SDL_Rect sprite_font_char_rect(const sprite_font_t *sprite_font, char x)
{
    assert(sprite_font);

    if (32 <= x && x <= 126) {
        const SDL_Rect rect = {
            .x = ((x - 32) % FONT_ROW_SIZE) * CHAR_WIDTH,
            .y = ((x - 32) / FONT_ROW_SIZE) * CHAR_HEIGHT,
            .w = CHAR_WIDTH,
            .h = CHAR_HEIGHT
        };
        return rect;
    } else {
        return sprite_font_char_rect(sprite_font, '?');
    }
}

int sprite_font_render_text(const sprite_font_t *sprite_font,
                            SDL_Renderer *renderer,
                            vec_t position,
                            int size,
                            color_t color,
                            const char *text)
{
    assert(sprite_font);
    assert(renderer);
    assert(text);

    const SDL_Color sdl_color = color_for_sdl(color);

    if (SDL_SetTextureColorMod(sprite_font->texture, sdl_color.r, sdl_color.g, sdl_color.b) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    const size_t text_size = strlen(text);
    const int px = (int) roundf(position.x);
    const int py = (int) roundf(position.y);
    for (size_t i = 0; i < text_size; ++i) {
        const SDL_Rect char_rect = sprite_font_char_rect(sprite_font, text[i]);
        const SDL_Rect dest_rect = {
            .x = px + CHAR_WIDTH * (int) i * size,
            .y = py,
            .w = char_rect.w * size,
            .h = char_rect.h * size
        };
        if (SDL_RenderCopy(renderer, sprite_font->texture, &char_rect, &dest_rect) < 0) {
            return -1;
        }
    }
    return 0;
}
