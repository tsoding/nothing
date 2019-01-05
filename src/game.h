#ifndef GAME_H_
#define GAME_H_

#include <SDL2/SDL.h>

#include "game/sound_samples.h"
#include "ebisp/expr.h"

typedef struct Game Game;

Game *create_game(const char *platforms_file_path,
                    const char *sound_sample_files[],
                    size_t sound_sample_files_count,
                    SDL_Renderer *renderer);
void destroy_game(Game *game);

int game_render(const Game *game);
int game_sound(Game *game);
int game_update(Game *game, float delta_time);

int game_event(Game *game, const SDL_Event *event);
int game_input(Game *game,
               const Uint8 *const keyboard_state,
               SDL_Joystick *the_stick_of_joy);

int game_over_check(const Game *game);

struct EvalResult
game_send(Game *game, Gc *gc, struct Scope *scope, struct Expr path);

#endif  // GAME_H_
