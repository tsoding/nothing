#include "boxes.h"
#include "platforms.h"
#include "player/rigid_rect.h"
#include "solid.h"

void solid_rect_object_collide(solid_ref_t solid,
                               rect_t object,
                               int sides[RECT_SIDE_N])
{
    switch (solid.tag) {
    case SOLID_PLATFORMS:
        platforms_rect_object_collide((platforms_t *) solid.ptr, object, sides);
        break;

    case SOLID_BOXES:
        boxes_rect_object_collide((boxes_t *) solid.ptr, object, sides);
        break;

    case SOLID_RIGID_RECT:
        rigid_body_object_collide((rigid_rect_t *) solid.ptr, object, sides);
        break;

    default: {}
    }
}
