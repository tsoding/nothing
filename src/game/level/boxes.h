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

solid_ref_t boxes_as_solid(boxes_t *boxes);

int boxes_render(boxes_t *boxes, camera_t *camera);
int boxes_update(boxes_t *boxes, float delta_time);
void boxes_touches_rect_sides(const boxes_t *boxes,
                              rect_t object,
                              int sides[RECT_SIDE_N]);

void boxes_collide_with_solid(boxes_t *boxes,
                              solid_ref_t solid);
void boxes_collide_with_lava(boxes_t *boxes,
                             lava_t *lava);

int boxes_add_to_physical_world(const boxes_t *boxes,
                                physical_world_t *physical_world_t);

/** \brief Implements solid_apply_force */
void boxes_apply_force(boxes_t *boxes,
                       vec_t force);

#endif  // BOXES_H_
