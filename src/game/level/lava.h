#ifndef LAVA_H_
#define LAVA_H_

#include <stdbool.h>

#include "game/camera.h"
#include "math/rect.h"

typedef struct lava_t lava_t;
typedef struct rigid_rect_t rigid_rect_t;

lava_t *create_lava_from_stream(FILE *stream);
void destroy_lava(lava_t *lava);

int lava_render(const lava_t *lava,
                camera_t *camera);
int lava_update(lava_t *lava, float delta_time);

bool lava_overlaps_rect(const lava_t *lava, rect_t rect);

void lava_float_rigid_rect(lava_t *lava, rigid_rect_t *rigid_rect);

#endif  // LAVA_H_
