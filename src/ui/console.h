#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <SDL2/SDL.h>
#include "ebisp/interpreter.h"

typedef struct Console Console;
typedef struct Level Level;
typedef struct Sprite_font Sprite_font;
typedef struct Camera Camera;

Console *create_console(Level *level,
                        const Sprite_font *font);
void destroy_console(Console *console);

int console_handle_event(Console *console,
                         const SDL_Event *event);

int console_render(const Console *console,
                   SDL_Renderer *renderer);

int console_update(Console *console,
                   float delta_time);

void console_slide_down(Console *console);

#endif  // CONSOLE_H_
