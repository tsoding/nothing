#ifndef BACKGROUND_H_
#define BACKGROUND_H_

#include <SDL.h>

#include "color.h"
#include "game/camera.h"

typedef struct {
    Color base_color;
} Background;

static inline
Background create_background(Color base_color)
{
    Background result = {base_color};
    return result;
}

int background_render(const Background *background,
                      const Camera *camera);

Color background_base_color(const Background *background);

#endif  // BACKGROUND_H_
