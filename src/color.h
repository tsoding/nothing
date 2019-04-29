#ifndef COLOR_H_
#define COLOR_H_

#include <SDL2/SDL.h>

typedef struct Color {
    float r, g, b, a;
} Color;

Color rgba(float r, float g, float b, float a);
Color hexstr(const char *hexstr);
SDL_Color color_for_sdl(Color color);

Color color_darker(Color color, float d);

Color color_desaturate(Color color);

Color color_invert(Color c);

#endif  // COLOR_H_
