#ifndef DYING_RECT_H_
#define DYING_RECT_H_

#include "./rect.h"
#include "./color.h"
#include "./camera.h"

typedef struct SDL_Renderer SDL_Renderer;
typedef struct dying_rect_t dying_rect_t;

dying_rect_t *create_dying_rect(rect_t rect,
                                color_t color,
                                Uint32 duration);
void destroy_dying_rect(dying_rect_t *dying_rect);

int dying_rect_render(const dying_rect_t *dying_rect,
                      SDL_Renderer *renderer,
                      const camera_t *camera);
int dying_rect_update(dying_rect_t *dying_rect,
                      Uint32 delta_time);

int dying_rect_is_dead(const dying_rect_t *dying_rect);

#endif  // DYING_RECT_H_
