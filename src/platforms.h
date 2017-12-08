#ifndef PLATFORMS_H_
#define PLATFORMS_H_

#include "./camera.h"
#include "./rect.h"

typedef struct platforms_t platforms_t;
typedef struct SDL_Renderer SDL_Renderer;

platforms_t *create_platforms(const rect_t *rects, size_t rects_size);
void destroy_platforms(platforms_t *platforms);

int render_platforms(const platforms_t *platforms,
                     SDL_Renderer *renderer,
                     const camera_t *camera);

void platforms_rect_object_collide(const platforms_t *platforms,
                                   const rect_t *object,
                                   int *sides);

#endif  // PLATFORMS_H_
