#include "boxes.h"
#include "platforms.h"
#include "player.h"
#include "player/rigid_rect.h"
#include "solid.h"

void solid_touches_rect_sides(Solid_ref solid,
                              Rect object,
                              int sides[RECT_SIDE_N])
{
    switch (solid.tag) {
    case SOLID_PLATFORMS:
        platforms_touches_rect_sides((Platforms *) solid.ptr, object, sides);
        break;

    case SOLID_RIGID_RECT:
        rigid_rect_touches_rect_sides((Rigid_rect *) solid.ptr, object, sides);
        break;
    }
}

void solid_apply_force(Solid_ref solid,
                       Vec force)
{
    switch (solid.tag) {
    case SOLID_PLATFORMS:
        /* no implementation */
        break;

    case SOLID_RIGID_RECT:
        rigid_rect_apply_force((Rigid_rect *) solid.ptr, force);
        break;

    default: {}
    }
}

void solid_collide_with_solid(Solid_ref solid,
                              Solid_ref other_solid)
{
    switch (solid.tag) {
    case SOLID_PLATFORMS:
        /* no implementation */
        break;

    case SOLID_RIGID_RECT:
        rigid_rect_collide_with_solid((Rigid_rect *) solid.ptr, other_solid);
        break;

    default: {}
    }
}
