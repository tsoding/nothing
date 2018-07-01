#ifndef RIGID_BODIES_H_
#define RIGID_BODIES_H_

#include "math/rect.h"

typedef struct rigid_bodies_t rigid_bodies_t;
typedef int rigid_rect_ref;

rigid_bodies_t *create_rigid_bodies(void);
void destroy_rigid_bodies(rigid_bodies_t *rigid_bodies);

void rigid_bodies_update(rigid_bodies_t *rigid_bodies,
                         float delta_time);

rigid_rect_ref rigid_bodies_create_rect(rigid_bodies_t *rigid_bodies,
                                        rect_t rect);
void rigid_bodies_destroy_rect(rigid_bodies_t *rigid_bodies,
                               rigid_rect_ref rect_ref);

#endif  // RIGID_BODIES_H_
