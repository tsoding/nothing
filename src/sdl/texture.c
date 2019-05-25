#include <SDL.h>

#include "system/stacktrace.h"
#include "system/log.h"
#include "texture.h"

SDL_Texture *texture_from_bmp(const char *bmp_file_name,
                              SDL_Renderer *renderer)
{
    trace_assert(bmp_file_name);
    trace_assert(renderer);

    SDL_Surface * surface = SDL_LoadBMP(bmp_file_name);
    if (surface == NULL) {
        log_fail("Could not load %s: %s\n", bmp_file_name, SDL_GetError());
        goto fail;
    }

    if (SDL_SetColorKey(surface,
                        SDL_TRUE,
                        SDL_MapRGB(surface->format, 0, 0, 0)) < 0) {
        log_fail("SDL_SetColorKey: %s\n", SDL_GetError());
        goto fail;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        log_fail("SDL_CreateTextureFromSurface: %s\n", SDL_GetError());
        goto fail;
    }

    SDL_FreeSurface(surface);

    return texture;

fail:
    if (surface != NULL) {
        SDL_FreeSurface(surface);
    }

    return NULL;
}
