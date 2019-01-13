#ifndef PHYSICAL_WORLD_H_
#define PHYSICAL_WORLD_H_

#include "game/level/solid.h"

// TODO(#632): remove Physical_world when it's not used anywhere anymore

typedef struct Physical_world Physical_world;

Physical_world *create_physical_world(void);
void destroy_physical_world(Physical_world *physical_world);

void physical_world_apply_gravity(Physical_world *physical_world);
void physical_world_collide_solids(Physical_world *physical_world,
                                   Platforms *platforms);
int physical_world_add_solid(Physical_world *physical_world,
                             Solid_ref solid);
void physical_world_clean(Physical_world *physical_world);

#endif  // PHYSICAL_WORLD_H_
