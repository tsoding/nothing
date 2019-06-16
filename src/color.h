#ifndef COLOR_H_
#define COLOR_H_

#include <SDL.h>

typedef struct Color {
    float r, g, b, a;
} Color;

Color rgba(float r, float g, float b, float a);
Color hexstr(const char *hexstr);
SDL_Color color_for_sdl(Color color);

int color_hex_to_stream(Color color, FILE *stream);

Color color_darker(Color color, float d);

Color color_desaturate(Color color);

Color color_invert(Color c);

Color color_scale(Color c, Color fc);

#endif  // COLOR_H_
