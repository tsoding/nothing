#ifndef LAVA_H_
#define LAVA_H_

#include <stdbool.h>

#include "game/camera.h"
#include "math/rect.h"

typedef struct Lava Lava;
typedef struct Rigid_rect Rigid_rect;
typedef struct LineStream LineStream;

Lava *create_lava_from_line_stream(LineStream *line_stream);
void destroy_lava(Lava *lava);

int lava_render(const Lava *lava,
                Camera *camera);
int lava_update(Lava *lava, float delta_time);

bool lava_overlaps_rect(const Lava *lava, Rect rect);

void lava_float_rigid_rect(Lava *lava, Rigid_rect *rigid_rect);

#endif  // LAVA_H_
