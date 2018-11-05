#ifndef CONSOLE_LOG_H_
#define CONSOLE_LOG_H_

#include "math/point.h"

typedef struct Console_Log Console_Log;

Console_Log *create_console_log(const Sprite_font *font,
                                Vec font_size,
                                size_t capacity);
void destroy_console_log(Console_Log *console_log);

int console_log_render(const Console_Log *console_log,
                       SDL_Renderer *renderer,
                       Point position);

int console_log_push_line(Console_Log *console_log,
                          const char *line,
                          Color color);

#endif  // CONSOLE_LOG_H_
