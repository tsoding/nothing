#ifndef BOXES_H_
#define BOXES_H_

#include "game/camera.h"
#include "game/level/platforms.h"
#include "lava.h"

typedef struct boxes_t boxes_t;
typedef struct player_t player_t;
typedef struct physical_world_t physical_world_t;

boxes_t *create_boxes_from_stream(FILE *stream);
void destroy_boxes(boxes_t *boxes);

int boxes_render(boxes_t *boxes, camera_t *camera);
int boxes_update(boxes_t *boxes, float delta_time);

int boxes_add_to_physical_world(const boxes_t *boxes,
                                physical_world_t *physical_world_t);

#endif  // BOXES_H_
