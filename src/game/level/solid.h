#ifndef SOLID_H_
#define SOLID_H_

#include "math/rect.h"

typedef enum solid_tag_t {
    SOLID_PLATFORMS = 0,
    SOLID_BOXES
} solid_tag_t;

typedef struct solid_ref_t {
    solid_tag_t tag;
    void *ptr;
} solid_ref_t;

void solid_rect_object_collide(solid_ref_t solid,
                               rect_t object,
                               int sides[RECT_SIDE_N]);

#endif  // SOLID_H_
