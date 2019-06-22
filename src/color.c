#include <SDL.h>
#include <string.h>

#include "color.h"

Color rgba(float r, float g, float b, float a)
{
    const Color result = {
        .r = r,
        .g = g,
        .b = b,
        .a = a
    };

    return result;
}

static Uint8 hex2dec_digit(char c)
{
    if (c >= '0' && c <= '9') {
        return (Uint8) (c - '0');
    }

    if (c >= 'A' && c <= 'F') {
        return (Uint8) (10 + c - 'A');
    }

    if (c >= 'a' && c <= 'f') {
        return (Uint8) (10 + c - 'a');
    }

    return 0;
}

static Uint8 parse_color_component(const char *component)
{
    return (Uint8) (hex2dec_digit(component[0]) * 16 + hex2dec_digit(component[1]));
}

Color hexstr(const char *hexstr)
{
    if (strlen(hexstr) != 6) {
        return rgba(0.0f, 0.0f, 0.0f, 1.0f);
    }

    return rgba(
        parse_color_component(hexstr) / 255.0f,
        parse_color_component(hexstr + 2) / 255.0f,
        parse_color_component(hexstr + 4) / 255.0f,
        1.0f);
}

SDL_Color color_for_sdl(Color color)
{
    const SDL_Color result = {
        .r = (Uint8)roundf(color.r * 255.0f),
        .g = (Uint8)roundf(color.g * 255.0f),
        .b = (Uint8)roundf(color.b * 255.0f),
        .a = (Uint8)roundf(color.a * 255.0f)
    };

    return result;
}

Color color_desaturate(Color c)
{
    const float k = (c.r + c.g + c.b) / 3.0f;
    return rgba(k, k, k, c.a);
}

Color color_darker(Color c, float d)
{
    return rgba(fmaxf(c.r - d, 0.0f),
                fmaxf(c.g - d, 0.0f),
                fmaxf(c.b - d, 0.0f),
                c.a);
}

Color color_invert(Color c)
{
    return rgba(1.0f - c.r, 1.0f - c.g, 1.0f - c.b, c.a);
}

Color color_scale(Color c, Color fc)
{
    return rgba(
        fmaxf(fminf(c.r * fc.r, 1.0f), 0.0f),
        fmaxf(fminf(c.g * fc.g, 1.0f), 0.0f),
        fmaxf(fminf(c.b * fc.b, 1.0f), 0.0f),
        fmaxf(fminf(c.a * fc.a, 1.0f), 0.0f));
}

int color_hex_to_stream(Color color, FILE *stream)
{
    SDL_Color sdl = color_for_sdl(color);
    return fprintf(stream, "%02x%02x%02x", sdl.r, sdl.g, sdl.b);
}
