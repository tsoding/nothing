#ifndef BACKGROUND_H_
#define BACKGROUND_H_

#include <SDL.h>

#include "color.h"
#include "game/camera.h"

typedef struct Background Background;
typedef struct LineStream LineStream;

Background *create_background(Color base_color);
Background *create_background_from_line_stream(LineStream *line_stream);
void destroy_background(Background *background);

int background_render(const Background *background,
                      Camera *camera);

void background_toggle_debug_mode(Background *background);

#endif  // BACKGROUND_H_
