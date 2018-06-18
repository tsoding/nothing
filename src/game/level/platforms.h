#ifndef PLATFORMS_H_
#define PLATFORMS_H_

#include <SDL2/SDL.h>

#include "game/camera.h"
#include "math/rect.h"
#include "solid.h"

typedef struct platforms_t platforms_t;

platforms_t *create_platforms_from_stream(FILE *stream);
platforms_t *create_platforms_from_file(const char *filename);
void destroy_platforms(platforms_t *platforms);

solid_ref_t platforms_as_solid(platforms_t *platforms);

int platforms_save_to_file(const platforms_t *platforms,
                           const char *filename);

int platforms_render(const platforms_t *platforms,
                     const camera_t *camera);

void platforms_touches_rect_sides(const platforms_t *platforms,
                                  rect_t object,
                                  int sides[RECT_SIDE_N]);

/** \brief implements solid_apply_force
 */
void platforms_apply_force(platforms_t *platforms,
                           vec_t force);

#endif  // PLATFORMS_H_
