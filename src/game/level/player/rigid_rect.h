#ifndef RIGID_RECT_H_
#define RIGID_RECT_H_

#include "color.h"
#include "game/camera.h"
#include "game/level/lava.h"
#include "game/level/platforms.h"
#include "game/level/solid.h"
#include "math/mat3x3.h"
#include "math/rect.h"

// TODO(#215): replace rigid_rect_t with rigid_bodies_t

typedef struct rigid_rect_t rigid_rect_t;
typedef struct boxes_t boxes_t;

rigid_rect_t *create_rigid_rect(rect_t rect, color_t color);
rigid_rect_t *create_rigid_rect_from_stream(FILE *stream);
void destroy_rigid_rect(rigid_rect_t *rigid_rect);

solid_ref_t rigid_rect_as_solid(rigid_rect_t *rigid_rect);

int rigid_rect_render(const rigid_rect_t *rigid_rect,
                      camera_t *camera);
int rigid_rect_update(rigid_rect_t * rigid_rect,
                      float delta_time);

void rigid_rect_touches_rect_sides(rigid_rect_t *rigid_rect,
                                   rect_t object,
                                   int sides[RECT_SIDE_N]);

void rigid_rect_collide_with_solid(rigid_rect_t * rigid_rect,
                                   solid_ref_t solid);
void rigid_rect_collide_with_lava(rigid_rect_t *rigid_rect,
                                  lava_t *lava);

rect_t rigid_rect_hitbox(const rigid_rect_t *rigid_rect);

void rigid_rect_move(rigid_rect_t *rigid_rect,
                     vec_t movement);

int rigid_rect_touches_ground(const rigid_rect_t *rigid_rect);

/** \brief implements solid_apply_force
 */
void rigid_rect_apply_force(rigid_rect_t * rigid_rect,
                            vec_t force);

void rigid_rect_transform_velocity(rigid_rect_t *rigid_rect,
                                   mat3x3 trans_mat);

#endif  // RIGID_RECT_H_
