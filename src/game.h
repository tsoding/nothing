#ifndef GAME_H_
#define GAME_H_

#include <SDL2/SDL.h>

#include "game/sound_samples.h"

typedef struct game_t game_t;

game_t *create_game(const char *platforms_file_path,
                    const char *sound_sample_files[],
                    size_t sound_sample_files_count,
                    SDL_Renderer *renderer);
void destroy_game(game_t *game);

int game_render(const game_t *game);
int game_sound(game_t *game);
int game_update(game_t *game, float delta_time);

int game_event(game_t *game, const SDL_Event *event);
int game_input(game_t *game,
               const Uint8 *const keyboard_state,
               SDL_Joystick *the_stick_of_joy);

int game_over_check(const game_t *game);

#endif  // GAME_H_
