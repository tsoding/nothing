#ifndef COLOR_H_
#define COLOR_H_

#include <stdio.h>
#include <SDL.h>
#include "./system/s.h"

#define COLOR_BLACK rgba(0.0f, 0.0f, 0.0f, 1.0f)
#define COLOR_WHITE rgba(1.0f, 1.0f, 1.0f, 1.0f)
#define COLOR_RED rgba(1.0f, 0.0f, 0.0f, 1.0f)

typedef struct Color {
    float r, g, b, a;
} Color;

Color rgba(float r, float g, float b, float a);
Color hsla(float h, float s, float l, float a);
Color rgba_to_hsla(Color color);
Color hexstr(const char *hexstr);
Color hexs(String input);
SDL_Color color_for_sdl(Color color);

int color_hex_to_stream(Color color, FILE *stream);
int color_hex_to_string(Color color, char *buffer, size_t buffer_size);

Color color_darker(Color color, float d);

Color color_desaturate(Color color);

Color color_invert(Color c);

Color color_scale(Color c, Color fc);

#endif  // COLOR_H_
