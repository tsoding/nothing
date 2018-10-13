#include <assert.h>

#include "region.h"
#include "player.h"

Region *create_region(Rect rect, const char *script_src)
{
    assert(script_src);
    (void) rect;
    return NULL;
}

void destroy_region(Region *region)
{
    assert(region);
}

void region_player_enter(Region *region, Player *player)
{
    assert(region);
    assert(player);
}

void region_player_leave(Region *region, Player *player)
{
    assert(region);
    assert(player);
}
