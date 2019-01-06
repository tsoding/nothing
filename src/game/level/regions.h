#ifndef REGIONS_H_
#define REGIONS_H_

#include "math/rect.h"

typedef struct Regions Regions;
typedef struct Player Player;
typedef struct LineStream LineStream;
typedef struct Level Level;
typedef struct Camera Camera;

Regions *create_regions_from_line_stream(LineStream *line_stream, Broadcast *broadcast);
void destroy_regions(Regions *regions);

int regions_render(Regions *regions, Camera *camera);

void regions_player_enter(Regions *regions, Player *player);
void regions_player_leave(Regions *regions, Player *player);

#endif  // REGIONS_H_
