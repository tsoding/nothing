#ifndef PHANTOM_PLATFORMS_H_
#define PHANTOM_PLATFORMS_H_

#include <stdlib.h>
#include "math/rect.h"
#include "color.h"
#include "game/level/level_editor/rect_layer.h"

typedef struct {
    size_t size;
    Rect *rects;
    Color *colors;
    int *hiding;
} Phantom_Platforms;

Phantom_Platforms create_phantom_platforms(RectLayer *rect_layer);
void destroy_phantom_platforms(Phantom_Platforms pp);

void phantom_platforms_render(const Phantom_Platforms *pp, const Camera *camera);
void phantom_platforms_update(Phantom_Platforms *pp, float dt);
void phantom_platforms_hide_at(Phantom_Platforms *pp, Vec2f position);

#endif  // PHANTOM_PLATFORMS_H_
