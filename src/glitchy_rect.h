#ifndef GLITCHY_RECT_H_
#define GLITCHY_RECT_H_

#include "./rect.h"
#include "./color.h"

typedef struct glitchy_rect_t glitchy_rect_t;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct camera_t camera_t;

glitchy_rect_t *create_glitchy_rect(rect_t rect, color_t color);
glitchy_rect_t *create_glitchy_rect_from_stream(FILE *stream);
void destroy_glitchy_rect(glitchy_rect_t *glitchy_rect);

int glitchy_rect_render(const glitchy_rect_t *glitchy_rect,
                        SDL_Renderer *renderer,
                        const camera_t *camera);
int glitchy_rect_update(glitchy_rect_t *glitchy_rect,
                        Uint32 delta_time);

int glitchy_rect_overlaps(const glitchy_rect_t *glitchy_rect,
                          rect_t rect);

#endif  // GLITCHY_RECT_H_
