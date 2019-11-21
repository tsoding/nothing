#ifndef CREDITS_H_
#define CREDITS_H_

#include <SDL.h>

#include "game/camera.h"

typedef struct Credits Credits;

Credits *create_credits(void);
void destroy_credits(Credits *credits);

int credits_render(const Credits *credits,
                   const Camera *camera);
int credits_update(Credits *credits,
				   Camera *camera, float dt);

#endif  // CREDITS_H_