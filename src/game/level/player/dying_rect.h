#ifndef DYING_RECT_H_
#define DYING_RECT_H_

#include <SDL2/SDL.h>

#include "color.h"
#include "game/camera.h"
#include "math/rect.h"

// TODO(#691): Dying_rect should be called Explosion

typedef struct Dying_rect Dying_rect;

Dying_rect *create_dying_rect(Color color,
                                float duration);
void destroy_dying_rect(Dying_rect *dying_rect);

int dying_rect_render(const Dying_rect *dying_rect,
                      Camera *camera);
int dying_rect_update(Dying_rect *dying_rect,
                      float delta_time);

int dying_rect_is_dead(const Dying_rect *dying_rect);

void dying_rect_start_dying(Dying_rect *dying_rect,
                            Vec position);

#endif  // DYING_RECT_H_
