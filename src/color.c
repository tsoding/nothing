#include <SDL2/SDL.h>

#include "./color.h"

color_t color(float r, float g, float b, float a)
{
    const color_t result = {
        .r = r,
        .g = g,
        .b = b,
        .a = a
    };

    return result;
}

SDL_Color color_for_sdl(color_t color)
{
    const SDL_Color result = {
        .r = (Uint8)roundf(color.r * 255.0f),
        .g = (Uint8)roundf(color.g * 255.0f),
        .b = (Uint8)roundf(color.b * 255.0f),
        .a = (Uint8)roundf(color.a * 255.0f)
    };

    return result;
}
