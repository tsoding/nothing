#ifndef LOG_H_
#define LOG_H_

#include "math/point.h"

// TODO(#481): ui/log unit may collide with system/log unit

typedef struct Log Log;

Log *create_log(const Sprite_font *font,
                Vec font_size,
                size_t capacity);
void destroy_log(Log *log);

int log_render(const Log *log,
               SDL_Renderer *renderer,
               Point position);

int log_push_line(Log *log,
                  const char *line,
                  Color color);

#endif  // LOG_H_
