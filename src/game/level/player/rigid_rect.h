#ifndef RIGID_RECT_H_
#define RIGID_RECT_H_

#include "color.h"
#include "game/camera.h"
#include "game/level/lava.h"
#include "game/level/platforms.h"
#include "math/mat3x3.h"
#include "math/rect.h"
#include "ebisp/expr.h"

// TODO(#634): remove Rigid_rect when it's not used anywhere anymore

typedef struct Rigid_rect Rigid_rect;
typedef struct Boxes Boxes;
typedef struct LineStream LineStream;

Rigid_rect *create_rigid_rect(Rect rect, Color color, const char *id);
Rigid_rect *create_rigid_rect_from_line_stream(LineStream *line_stream);
void destroy_rigid_rect(Rigid_rect *rigid_rect);

int rigid_rect_render(const Rigid_rect *rigid_rect,
                      Camera *camera);
int rigid_rect_update(Rigid_rect * rigid_rect,
                      float delta_time);

void rigid_rect_touches_rect_sides(Rigid_rect *rigid_rect,
                                   Rect object,
                                   int sides[RECT_SIDE_N]);

Rect rigid_rect_hitbox(const Rigid_rect *rigid_rect);

void rigid_rect_move(Rigid_rect *rigid_rect,
                     Vec movement);

int rigid_rect_touches_ground(const Rigid_rect *rigid_rect);

/** \brief implements solid_apply_force
 */
void rigid_rect_apply_force(Rigid_rect * rigid_rect,
                            Vec force);

void rigid_rect_transform_velocity(Rigid_rect *rigid_rect,
                                   mat3x3 trans_mat);

void rigid_rect_teleport_to(Rigid_rect *rigid_rect,
                            Vec position);

void rigid_rect_damper(Rigid_rect *rigid_rect,
                       Vec v);

bool rigid_rect_has_id(Rigid_rect *rigid_rect,
                       const char *id);

struct EvalResult
rigid_rect_send(Rigid_rect *rigid_rect, Gc *gc, struct Scope *scope, struct Expr path);

#endif  // RIGID_RECT_H_
