#ifndef LEVEL_H_
#define LEVEL_H_

#include <SDL.h>

#include "game/camera.h"
#include "game/level/platforms.h"
#include "game/level/player.h"
#include "sound_samples.h"

typedef struct Level Level;
typedef struct LevelEditor LevelEditor;

Level *create_level_from_level_editor(const LevelEditor *level_editor);
void destroy_level(Level *level);

int level_render(const Level *level, const Camera *camera);

int level_sound(Level *level, Sound_samples *sound_samples);
int level_update(Level *level, float delta_time);

int level_event(Level *level, const SDL_Event *event,
                Camera *camera, Sound_samples *sound_samples);
int level_input(Level *level,
                const Uint8 *const keyboard_state,
                SDL_Joystick *the_stick_of_joy);
int level_enter_camera_event(Level *level, Camera *camera);

void level_disable_pause_mode(Level *level, Camera *camera,
                              Sound_samples *sound_samples);

#endif  // LEVEL_H_
