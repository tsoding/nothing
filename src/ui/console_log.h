#ifndef CONSOLE_LOG_H_
#define CONSOLE_LOG_H_

#include "math/vec.h"
#include "game/camera.h"

typedef struct Console_Log Console_Log;

Console_Log *create_console_log(Vec2f font_size,
                                size_t capacity);
void destroy_console_log(Console_Log *console_log);

void console_log_render(const Console_Log *console_log,
                        const Camera *camera,
                        Vec2f position);

int console_log_push_line(Console_Log *console_log,
                          const char *line,
                          const char *line_end,
                          Color color);

void console_log_clear(Console_Log *console_log);

#endif  // CONSOLE_LOG_H_
