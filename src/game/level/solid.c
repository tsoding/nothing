#include "boxes.h"
#include "platforms.h"
#include "player.h"
#include "player/rigid_rect.h"
#include "solid.h"

void solid_touches_rect_sides(solid_ref_t solid,
                              rect_t object,
                              int sides[RECT_SIDE_N])
{
    switch (solid.tag) {
    case SOLID_PLATFORMS:
        platforms_touches_rect_sides((platforms_t *) solid.ptr, object, sides);
        break;

    case SOLID_RIGID_RECT:
        rigid_rect_touches_rect_sides((rigid_rect_t *) solid.ptr, object, sides);
        break;

    case SOLID_PLAYER:
        player_touches_rect_sides((player_t *) solid.ptr, object, sides);
        break;
    }
}

void solid_apply_force(solid_ref_t solid,
                       vec_t force)
{
    switch (solid.tag) {
    case SOLID_PLATFORMS:
        /* no implementation */
        break;

    case SOLID_RIGID_RECT:
        rigid_rect_apply_force((rigid_rect_t *) solid.ptr, force);
        break;

    case SOLID_PLAYER:
        player_apply_force((player_t *) solid.ptr, force);
        break;

    default: {}
    }
}

void solid_collide_with_solid(solid_ref_t solid,
                              solid_ref_t other_solid)
{
    switch (solid.tag) {
    case SOLID_PLATFORMS:
        /* no implementation */
        break;

    case SOLID_RIGID_RECT:
        rigid_rect_collide_with_solid((rigid_rect_t *) solid.ptr, other_solid);
        break;

    case SOLID_PLAYER:
        player_collide_with_solid((player_t *) solid.ptr, other_solid);
        break;

    default: {}
    }
}
