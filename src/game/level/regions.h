#ifndef REGIONS_H_
#define REGIONS_H_

#include "math/rect.h"

typedef struct Regions Regions;
typedef struct Player Player;
typedef struct LineStream LineStream;

Regions *create_regions(Rect rect, const char *script_src);
Regions *create_regions_from_stream(LineStream *line_stream);
void destroy_regions(Regions *regions);

void regions_player_enter(Regions *regions, Player *player);
void regions_player_leave(Regions *regions, Player *player);

#endif  // REGIONS_H_
