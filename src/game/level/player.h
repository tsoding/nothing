#ifndef PLAYER_H_
#define PLAYER_H_

#include <SDL2/SDL.h>

#include "game/camera.h"
#include "game/sound_samples.h"
#include "lava.h"
#include "platforms.h"
#include "boxes.h"

typedef struct player_t player_t;
typedef struct goals_t goals_t;
typedef struct rigid_rect_t rigid_rect_t;

player_t *create_player(float x, float y, color_t color);
player_t *create_player_from_stream(FILE *stream);
void destroy_player(player_t * player);

solid_ref_t player_as_solid(player_t *player);

int player_render(const player_t * player,
                  camera_t *camera);
void player_update(player_t * player,
                   float delta_time);
void player_collide_with_solid(player_t *player, solid_ref_t solid);
void player_touches_rect_sides(player_t *player,
                               rect_t object,
                               int sides[RECT_SIDE_N]);

int player_sound(player_t *player,
                 sound_samples_t *sound_samples);
void player_checkpoint(player_t *player,
                       vec_t checkpoint);

void player_move_left(player_t *player);
void player_move_right(player_t *player);
void player_stop(player_t *player);
void player_jump(player_t *player);
void player_die(player_t *player);

void player_focus_camera(player_t *player,
                         camera_t *camera);
void player_hide_goals(const player_t *player,
                       goals_t *goal);
void player_die_from_lava(player_t *player,
                          const lava_t *lava);

/** \brief Implements solid_apply_force
 */
void player_apply_force(player_t *player, vec_t force);

#endif  // PLAYER_H_
