#ifndef RENDERER_H_
#define RENDERER_H_

#include <SDL2/SDL.h>

#include "color.h"
#include "math/point.h"
#include "math/triangle.h"

int draw_triangle(SDL_Renderer *render,
                  triangle_t t);

int fill_triangle(SDL_Renderer *render,
                  triangle_t t);

int fill_rect(SDL_Renderer *render,
              rect_t r,
              color_t c);

/* `getpixel()` and `putpixel()` were stolen from
 * https://www.libsdl.org/release/SDL-1.2.15/docs/html/guidevideo.html */
Uint32 getpixel(SDL_Surface *surface, int x, int y);
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

#endif  // RENDERER_H_
