#ifndef SOLID_H_
#define SOLID_H_

#include "math/rect.h"

// TODO(#636): remove solid interface when it's not used anywhere anymore

typedef enum Solid_tag {
    SOLID_PLATFORMS = 0,
    SOLID_RIGID_RECT
} Solid_tag;

typedef struct Solid_ref {
    Solid_tag tag;
    void *ptr;
} Solid_ref;

/** \brief Answers what sides of the rectangular object the solid entity touches
 */
void solid_touches_rect_sides(Solid_ref solid,
                              Rect object,
                              int sides[RECT_SIDE_N]);

/** \brief Applies a force vector to the solid body
 */
void solid_apply_force(Solid_ref solid,
                       Vec force);

void solid_collide_with_solid(Solid_ref solid,
                              Solid_ref other_solid);

#endif  // SOLID_H_
