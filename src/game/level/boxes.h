#ifndef BOXES_H_
#define BOXES_H_

#include "game/level/platforms.h"
#include "game/camera.h"

typedef struct boxes_t boxes_t;
typedef struct player_t player_t;

boxes_t *create_boxes_from_stream(FILE *stream);
void destroy_boxes(boxes_t *boxes);

int boxes_render(boxes_t *boxes, camera_t *camera);
int boxes_update(boxes_t *boxes, float delta_time);
void boxes_rect_object_collide(const boxes_t *boxes,
                               rect_t object,
                               int sides[RECT_SIDE_N]);
void boxes_collide_with_platforms(boxes_t *boxes,
                                  const platforms_t *platforms);
void boxes_collide_with_player(boxes_t *boxes,
                               player_t *player);


#endif  // BOXES_H_
