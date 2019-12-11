#ifndef BOXES_H_
#define BOXES_H_

#include "game/camera.h"
#include "game/level/platforms.h"
#include "lava.h"

typedef struct Boxes Boxes;
typedef struct Player Player;
typedef struct Player Player;
typedef struct RectLayer RectLayer;

Boxes *create_boxes_from_rect_layer(const RectLayer *layer, RigidBodies *rigid_bodies);
void destroy_boxes(Boxes *boxes);

int boxes_render(Boxes *boxes, const Camera *camera);
int boxes_update(Boxes *boxes, float delta_time);

void boxes_float_in_lava(Boxes *boxes, Lava *lava);

int boxes_add_box(Boxes *boxes, Rect rect, Color color);
int boxes_delete_at(Boxes *boxes, Vec2f position);

#endif  // BOXES_H_
