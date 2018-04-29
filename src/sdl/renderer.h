#ifndef RENDERER_H_
#define RENDERER_H_

#include <SDL2/SDL.h>

#include "math/point.h"
#include "math/triangle.h"

int draw_triangle(SDL_Renderer *render,
                  triangle_t t);

int fill_triangle(SDL_Renderer *render,
                  triangle_t t);


#endif  // RENDERER_H_
