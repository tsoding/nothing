#ifndef LEVEL_H_
#define LEVEL_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "game/camera.h"
#include "game/level/platforms.h"
#include "game/level/player.h"
#include "sound_samples.h"

typedef struct level_t level_t;

level_t *create_level_from_file(const char *file_name);
void destroy_level(level_t *level);

int level_render(const level_t *level, camera_t *camera);

int level_sound(level_t *level, sound_samples_t *sound_samples);
int level_update(level_t *level, float delta_time);

int level_event(level_t *level, const SDL_Event *event);
int level_input(level_t *level,
                const Uint8 *const keyboard_state,
                SDL_Joystick *the_stick_of_joy);
int level_enter_camera_event(level_t *level,
                             const camera_t *camera);

int level_reload_preserve_player(level_t *level,
                                 const char *file_name);

void level_toggle_debug_mode(level_t *level);
void level_toggle_pause_mode(level_t *level);

#endif  // LEVEL_H_
