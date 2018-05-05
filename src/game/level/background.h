#ifndef BACKGROUND_H_
#define BACKGROUND_H_

#include <SDL2/SDL.h>

#include "color.h"
#include "game/camera.h"

typedef struct background_t background_t;

background_t *create_background(color_t base_color);
void destroy_background(background_t *background);

int background_render(const background_t *background,
                      const camera_t *camera);

#endif  // BACKGROUND_H_
