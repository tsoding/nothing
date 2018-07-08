#ifndef RENDERER_H_
#define RENDERER_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "color.h"
#include "math/point.h"
#include "math/triangle.h"

int draw_triangle(SDL_Renderer *render,
                  triangle_t t);

int fill_triangle(SDL_Renderer *render,
                  triangle_t t);

int render_text_shaded(SDL_Renderer *render,
                       TTF_Font *font,
                       vec_t position,
                       color_t color,
                       color_t bg,
                       const char *text);

#endif  // RENDERER_H_
