#ifndef GAME_H_
#define GAME_H_

#include <SDL.h>

#include "game/sound_samples.h"

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

typedef enum Game_state {
    GAME_STATE_LEVEL = 0,
    GAME_STATE_LEVEL_PICKER,
    GAME_STATE_LEVEL_EDITOR,
    GAME_STATE_CREDITS,
    GAME_STATE_SETTINGS,
    GAME_STATE_QUIT
} Game_state;

void game_switch_state(Game *game, Game_state state);
int game_load_level(Game *game, const char *filepath);

// defined in main.c. is there a better place for this to be declared?
float get_display_scale(void);

#endif  // GAME_H_
