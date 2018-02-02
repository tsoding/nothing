#ifndef COLOR_H_
#define COLOR_H_

typedef struct SDL_Color SDL_Color;

typedef struct color_t {
    float r, g, b, a;
} color_t;

color_t color(float r, float g, float b, float a);
color_t color256(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
SDL_Color color_for_sdl(color_t color);

color_t color_desaturate(color_t color);

#endif  // COLOR_H_
