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

// TODO: Try to extend solid interface to recieve an impact feedback
void solid_rect_object_collide(solid_ref_t solid,
                               rect_t object,
                               int sides[RECT_SIDE_N]);

#endif  // SOLID_H_
