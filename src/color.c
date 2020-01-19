#include <SDL.h>
#include <string.h>
#include <math.h>

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

/* TODO(#928): Should the Hue in HSLA representation be in degrees? */
Color hsla(float h, float s, float l, float a)
{
    h = fmodf(h, 360.0f);

    const float c = (1.0f - fabsf(2.0f * l - 1.0f)) * s;
    const float x = c * (1 - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
    const float m = l - c / 2.0f;

    Color color = {0.0f, 0.0f, 0.0f, a};

    if (0.0f <= h && h < 60.0f) {
        color = rgba(c, x, 0.0f, a);
    } else if (60.0f <= h && h < 120.0f) {
        color = rgba(x, c, 0.0f, a);
    } else if (120.0f <= h && h < 180.0f) {
        color = rgba(0.0f, c, x, a);
    } else if (180.0f <= h && h < 240.0f) {
        color = rgba(0.0f, x, c, a);
    } else if (240.0f <= h && h < 300.0f) {
        color = rgba(x, 0.0f, c, a);
    } else if (300.0f <= h && h < 360.0f) {
        color = rgba(c, 0.0f, x, a);
    }

    color.r += m;
    color.g += m;
    color.b += m;

    return color;
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

Color hexs(String input)
{
    if (input.count < 6) return COLOR_BLACK;

    return rgba(
        parse_color_component(input.data) / 255.0f,
        parse_color_component(input.data + 2) / 255.0f,
        parse_color_component(input.data + 4) / 255.0f,
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

int color_hex_to_string(Color color, char *buffer, size_t buffer_size)
{
    SDL_Color sdl = color_for_sdl(color);
    return snprintf(buffer, buffer_size, "%02x%02x%02x", sdl.r, sdl.g, sdl.b);
}

Color rgba_to_hsla(Color color)
{
    const float max = fmaxf(color.r, fmaxf(color.g, color.b));
    const float min = fminf(color.r, fminf(color.g, color.b));
    const float c = max - min;
    float hue = 0.0f;
    float saturation = 0.0f;
    const float lightness = (max + min) * 0.5f;

    if (fabsf(c) > 1e-6) {
        if (fabs(max - color.r) <= 1e-6) {
            hue = 60.0f * fmodf((color.g - color.b) / c, 6.0f);
        } else if (fabs(max - color.g) <= 1e-6) {
            hue = 60.0f * ((color.b - color.r) / c + 2.0f);
        } else {
            hue = 60.0f * ((color.r - color.g) / c + 4.0f);
        }

        saturation = c / (1.0f - fabsf(2.0f * lightness - 1.0f));
    }

    // TODO(#929): Color struct is used not only for RGBA
    //   But also for HSLA. We should make another similar struct but for HSLA
    return rgba(hue, saturation, lightness, color.a);
}
