#include <SDL2/SDL.h>
#include <string.h>

#include "color.h"

Color color(float r, float g, float b, float a)
{
    const Color result = {
        .r = r,
        .g = g,
        .b = b,
        .a = a
    };

    return result;
}

Color color256(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    return color(
        (float) r / 255.0f,
        (float) g / 255.0f,
        (float) b / 255.0f,
        (float) a / 255.0f);
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

Color color_from_hexstr(const char *hexstr)
{
    if (strlen(hexstr) != 6) {
        return color(0.0f, 0.0f, 0.0f, 1.0f);
    }

    return color256(
        parse_color_component(hexstr),
        parse_color_component(hexstr + 2),
        parse_color_component(hexstr + 4),
        255);
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
    return color(k, k, k, c.a);
}

Color color_darker(Color c, float d)
{
    return color(fmaxf(c.r - d, 0.0f),
                 fmaxf(c.g - d, 0.0f),
                 fmaxf(c.b - d, 0.0f),
                 c.a);
}
