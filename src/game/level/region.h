#ifndef REGION_H_
#define REGION_H_

#include "math/rect.h"

typedef struct Region Region;
typedef struct Player Player;
typedef struct LineStream LineStream;

Region *create_region(Rect rect, const char *script_src);
Region *create_region_from_stream(LineStream *line_stream);
void destroy_region(Region *region);

void region_player_enter(Region *region, Player *player);
void region_player_leave(Region *region, Player *player);

#endif  // REGION_H_
