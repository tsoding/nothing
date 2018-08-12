#ifndef LAVA_H_
#define LAVA_H_

#include "game/camera.h"
#include "game/level/solid.h"
#include "math/rect.h"

typedef struct lava_t lava_t;

lava_t *create_lava_from_stream(FILE *stream);
void destroy_lava(lava_t *lava);

int lava_render(const lava_t *lava,
                camera_t *camera);
int lava_update(lava_t *lava, float delta_time);

int lava_overlaps_rect(const lava_t *lava, rect_t rect);

rect_t lava_overlap_area(const lava_t *lava, rect_t rect);

void lava_float_solid(const lava_t *lava, solid_ref_t solid);

#endif  // LAVA_H_
