#ifndef SOLID_H_
#define SOLID_H_

#include "math/rect.h"

typedef enum solid_tag_t {
    SOLID_PLATFORMS = 0,
    SOLID_BOXES,
    SOLID_RIGID_RECT,
    SOLID_PLAYER
} solid_tag_t;

typedef struct solid_ref_t {
    solid_tag_t tag;
    void *ptr;
} solid_ref_t;

/** \brief Answers what sides of the rectangular object the solid entity touches
 */
void solid_touches_rect_sides(solid_ref_t solid,
                              rect_t object,
                              int sides[RECT_SIDE_N]);

/** \brief Applies a force vector to the solid body
 */
void solid_apply_force(solid_ref_t solid,
                       vec_t force);

#endif  // SOLID_H_
