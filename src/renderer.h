#ifndef RENDERER_H_
#define RENDERER_H_

#include "./point.h"

typedef struct SDL_Renderer SDL_Renderer;

int draw_triangle(SDL_Renderer *render,
                  point_t p1,
                  point_t p2,
                  point_t p3);

int fill_triangle(SDL_Renderer *render,
                  point_t p1,
                  point_t p2,
                  point_t p3);


#endif  // RENDERER_H_
