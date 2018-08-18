#ifndef BOXES_H_
#define BOXES_H_

#include "game/camera.h"
#include "game/level/platforms.h"
#include "lava.h"

typedef struct Boxes Boxes;
typedef struct Player Player;
typedef struct Physical_world Physical_world;

Boxes *create_boxes_from_stream(FILE *stream);
void destroy_boxes(Boxes *boxes);

int boxes_render(Boxes *boxes, Camera *camera);
int boxes_update(Boxes *boxes, float delta_time);

void boxes_float_in_lava(Boxes *boxes, Lava *lava);

int boxes_add_to_physical_world(const Boxes *boxes,
                                Physical_world *Physical_world);

#endif  // BOXES_H_
