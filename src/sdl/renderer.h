#ifndef RENDERER_H_
#define RENDERER_H_

#include <SDL.h>

#include "color.h"
#include "math/vec.h"
#include "math/triangle.h"

// TODO(#474): there are no logging SDL wrappers (similar to system/nth_alloc)
int draw_triangle(SDL_Renderer *render,
                  Triangle t);

int fill_triangle(SDL_Renderer *render,
                  Triangle t);

int fill_rect(SDL_Renderer *render,
              Rect r,
              Color c);

#endif  // RENDERER_H_
