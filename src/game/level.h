#ifndef LEVEL_H_
#define LEVEL_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "game/camera.h"
#include "game/level/platforms.h"
#include "game/level/player.h"
#include "sound_samples.h"
#include "ebisp/expr.h"

typedef struct Broadcast Broadcast;
typedef struct Level Level;

Level *create_level_from_file(const char *file_name, Broadcast *broadcast);
void destroy_level(Level *level);

int level_render(const Level *level, Camera *camera);

int level_sound(Level *level, Sound_samples *sound_samples);
int level_update(Level *level, float delta_time);

int level_event(Level *level, const SDL_Event *event);
int level_input(Level *level,
                const Uint8 *const keyboard_state,
                SDL_Joystick *the_stick_of_joy);
int level_enter_camera_event(Level *level, Camera *camera);

int level_reload_preserve_player(Level *level,
                                 const char *file_name,
                                 Broadcast *broadcast);

Rigid_rect *level_rigid_rect(Level *level,
                             const char *rigid_rect_id);

void level_toggle_debug_mode(Level *level);
void level_toggle_pause_mode(Level *level);

struct EvalResult level_send(Level *level, Gc *gc, struct Scope *scope, struct Expr path);

#endif  // LEVEL_H_
