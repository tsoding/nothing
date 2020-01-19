#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <SDL.h>
#include "game/sprite_font.h"

typedef struct Console Console;
typedef struct Game Game;

Console *create_console(Game *game);
void destroy_console(Console *console);

int console_handle_event(Console *console,
                         const SDL_Event *event);

int console_render(const Console *console,
                   const Camera *camera);

int console_update(Console *console,
                   float delta_time);

void console_slide_down(Console *console);

#endif  // CONSOLE_H_
