#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <SDL2/SDL.h>

typedef struct Console Console;
typedef struct Level Level;
typedef struct Sprite_font Sprite_font;
typedef struct Camera Camera;

// TODO(#345): console does evaluate script expressions
// TODO(#346): console doesn't have any background
// TODO: console doesn't slide down when it's summoned

Console *create_console(Level *level,
                        const Sprite_font *font);
void destroy_console(Console *console);

int console_handle_event(Console *console,
                         const SDL_Event *event);

int console_render(const Console *console,
                   SDL_Renderer *renderer);

#endif  // CONSOLE_H_
