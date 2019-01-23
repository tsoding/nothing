#ifndef PLATFORMS_H_
#define PLATFORMS_H_

#include <SDL2/SDL.h>

#include "game/camera.h"
#include "math/rect.h"
#include "solid.h"

typedef struct Platforms Platforms;
typedef struct LineStream LineStream;

Platforms *create_platforms_from_line_stream(LineStream *line_stream);
void destroy_platforms(Platforms *platforms);

Solid_ref platforms_as_solid(Platforms *platforms);

int platforms_save_to_file(const Platforms *platforms,
                           const char *filename);

int platforms_render(const Platforms *platforms,
                     Camera *camera);

void platforms_touches_rect_sides(const Platforms *platforms,
                                  Rect object,
                                  int sides[RECT_SIDE_N]);
Rect platforms_snap_rect(const Platforms *platforms,
                         Rect object);

#endif  // PLATFORMS_H_
