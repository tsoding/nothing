#ifndef PHYSICAL_WORLD_H_
#define PHYSICAL_WORLD_H_

#include "game/level/solid.h"

typedef struct physical_world_t physical_world_t;

physical_world_t *create_physical_world(void);
void destroy_physical_world(physical_world_t *physical_world);

void physical_world_apply_gravity(physical_world_t *physical_world);
void physical_world_collide_solids(physical_world_t *physical_world,
                                   platforms_t *platforms);
int physical_world_add_solid(physical_world_t *physical_world,
                             solid_ref_t solid);
void physical_world_clean(physical_world_t *physical_world);

#endif  // PHYSICAL_WORLD_H_
