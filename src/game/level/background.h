#ifndef BACKGROUND_H_
#define BACKGROUND_H_

#include <SDL2/SDL.h>

#include "color.h"
#include "game/camera.h"

typedef struct background_t background_t;

background_t *create_background(color_t base_color);
void destroy_background(background_t *background);

int background_render(const background_t *background,
                      camera_t *camera);

void background_toggle_debug_mode(background_t *background);

#endif  // BACKGROUND_H_
