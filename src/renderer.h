#ifndef RENDERER_H_
#define RENDERER_H_

#include "./point.h"
#include "./triangle.h"

typedef struct SDL_Renderer SDL_Renderer;

int draw_triangle(SDL_Renderer *render,
                  triangle_t t);

int fill_triangle(SDL_Renderer *render,
                  triangle_t t);


#endif  // RENDERER_H_
