#ifndef PLATFORMS_H_
#define PLATFORMS_H_

#include <SDL2/SDL.h>

#include "./camera.h"
#include "./math/rect.h"

typedef struct platforms_t platforms_t;

platforms_t *create_platforms_from_stream(FILE *stream);
platforms_t *create_platforms_from_file(const char *filename);
void destroy_platforms(platforms_t *platforms);

int platforms_save_to_file(const platforms_t *platforms,
                           const char *filename);

int platforms_render(const platforms_t *platforms,
                     SDL_Renderer *renderer,
                     const camera_t *camera);

void platforms_rect_object_collide(const platforms_t *platforms,
                                   rect_t object,
                                   int sides[RECT_SIDE_N]);

#endif  // PLATFORMS_H_
