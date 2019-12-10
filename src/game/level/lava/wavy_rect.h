#ifndef WAVY_RECT_H_
#define WAVY_RECT_H_

#include <SDL.h>

#include "color.h"
#include "game/camera.h"
#include "math/rect.h"

typedef struct Wavy_rect Wavy_rect;

Wavy_rect *create_wavy_rect(Rect rect, Color color);
void destroy_wavy_rect(Wavy_rect *wavy_rect);

int wavy_rect_render(const Wavy_rect *wavy_rect,
                     const Camera *camera);
int wavy_rect_update(Wavy_rect *wavy_rect,
                     float delta_time);

Rect wavy_rect_hitbox(const Wavy_rect *wavy_rect);

#endif  // WAVY_RECT_H_
