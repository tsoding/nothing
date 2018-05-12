#ifndef BOXES_H_
#define BOXES_H_

#include "game/level/platforms.h"
#include "game/camera.h"

typedef struct boxes_t boxes_t;

boxes_t *create_boxes_from_stream(FILE *stream);
void destroy_boxes(boxes_t *boxes);

int boxes_render(boxes_t *boxes, camera_t *camera);
int boxes_update(boxes_t *boxes, float delta_time);
int boxes_collide_with_platforms(boxes_t *boxes, platforms_t *platforms);

#endif  // BOXES_H_
