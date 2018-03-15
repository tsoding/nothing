#ifndef WAVY_RECT_H_
#define WAVY_RECT_H_

#include "./rect.h"
#include "./color.h"
#include "./camera.h"

typedef struct wavy_rect_t wavy_rect_t;
typedef struct SDL_Renderer SDL_Renderer;

wavy_rect_t *create_wavy_rect(rect_t rect, color_t color);
wavy_rect_t *create_wavy_rect_from_stream(FILE *stream);
void destroy_wavy_rect(wavy_rect_t *wavy_rect);

int wavy_rect_render(const wavy_rect_t *wavy_rect,
                     SDL_Renderer *renderer,
                     const camera_t *camera);
int wavy_rect_update(wavy_rect_t *wavy_rect,
                     Uint32 delta_time);

int wavy_rect_overlaps(const wavy_rect_t *wavy_rect,
                       rect_t rect);

#endif  // WAVY_RECT_H_
