#ifndef PLATFORMS_H_
#define PLATFORMS_H_

#include "./rect.h"

struct platforms_t;
struct SDL_Renderer;

struct platforms_t *create_platforms(const struct rect_t *rects, size_t rects_size);
void destroy_platforms(struct platforms_t *platforms);

int render_platforms(const struct platforms_t *platforms,
                     SDL_Renderer *renderer);

int platforms_rect_object_collide(const struct platforms_t *platforms,
                                  const struct rect_t *object);

#endif  // PLATFORMS_H_
