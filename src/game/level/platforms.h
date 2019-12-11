#ifndef PLATFORMS_H_
#define PLATFORMS_H_

#include <SDL.h>

#include "game/camera.h"
#include "math/rect.h"

typedef struct Platforms Platforms;
typedef struct RectLayer RectLayer;

Platforms *create_platforms_from_rect_layer(const RectLayer *layer);
void destroy_platforms(Platforms *platforms);

int platforms_render(const Platforms *platforms,
                     const Camera *camera);

void platforms_touches_rect_sides(const Platforms *platforms,
                                  Rect object,
                                  int sides[RECT_SIDE_N]);
Vec2f platforms_snap_rect(const Platforms *platforms,
                          Rect *object);

#endif  // PLATFORMS_H_
