#ifndef EXPLOSION_H_
#define EXPLOSION_H_

#include <SDL.h>

#include "color.h"
#include "game/camera.h"
#include "math/rect.h"

typedef struct Explosion Explosion;

Explosion *create_explosion(Color color,
                                float duration);
void destroy_explosion(Explosion *explosion);

int explosion_render(const Explosion *explosion,
                     const Camera *camera);
int explosion_update(Explosion *explosion,
                     float delta_time);

int explosion_is_done(const Explosion *explosion);

void explosion_start(Explosion *explosion, Vec2f position);

#endif  // EXPLOSION_H_
