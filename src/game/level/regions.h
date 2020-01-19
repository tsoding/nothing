#ifndef REGIONS_H_
#define REGIONS_H_

#include "math/rect.h"
#include "action.h"

typedef struct Regions Regions;
typedef struct Player Player;
typedef struct Level Level;
typedef struct RectLayer RectLayer;
typedef struct Labels Labels;
typedef struct Goals Goals;

Regions *create_regions_from_rect_layer(const RectLayer *rect_layer, Labels *labels, Goals *goals);
void destroy_regions(Regions *regions);

int regions_render(Regions *regions, const Camera *camera);

void regions_player_enter(Regions *regions, Player *player);
void regions_player_leave(Regions *regions, Player *player);

#endif  // REGIONS_H_
