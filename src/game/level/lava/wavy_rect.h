#ifndef WAVY_RECT_H_
#define WAVY_RECT_H_

#include <SDL2/SDL.h>

#include "color.h"
#include "game/camera.h"
#include "math/rect.h"

typedef struct wavy_rect_t wavy_rect_t;

wavy_rect_t *create_wavy_rect(rect_t rect, color_t color);
wavy_rect_t *create_wavy_rect_from_stream(FILE *stream);
void destroy_wavy_rect(wavy_rect_t *wavy_rect);

int wavy_rect_render(const wavy_rect_t *wavy_rect,
                     const camera_t *camera);
int wavy_rect_update(wavy_rect_t *wavy_rect,
                     float delta_time);

int wavy_rect_overlaps(const wavy_rect_t *wavy_rect,
                       rect_t rect);

#endif  // WAVY_RECT_H_
