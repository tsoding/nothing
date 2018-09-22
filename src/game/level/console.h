#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <SDL2/SDL.h>

typedef struct Console Console;
typedef struct Level Level;
typedef struct Sprite_font Sprite_font;

Console *create_console(Level *level,
                        const Sprite_font *font);
void destroy_console(Console *console);

int console_handle_event(Console *console,
                         const SDL_Event *event);

#endif  // CONSOLE_H_
