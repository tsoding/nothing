#ifndef DYING_RECT_H_
#define DYING_RECT_H_

#include <SDL2/SDL.h>

#include "color.h"
#include "game/camera.h"
#include "math/rect.h"

typedef struct dying_rect_t dying_rect_t;

dying_rect_t *create_dying_rect(color_t color,
                                float duration);
void destroy_dying_rect(dying_rect_t *dying_rect);

int dying_rect_render(const dying_rect_t *dying_rect,
                      camera_t *camera);
int dying_rect_update(dying_rect_t *dying_rect,
                      float delta_time);

int dying_rect_is_dead(const dying_rect_t *dying_rect);

void dying_rect_start_dying(dying_rect_t *dying_rect,
                            vec_t position);

#endif  // DYING_RECT_H_
