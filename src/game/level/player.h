#ifndef PLAYER_H_
#define PLAYER_H_

#include <SDL.h>

#include "game/camera.h"
#include "game/sound_samples.h"
#include "lava.h"
#include "platforms.h"
#include "boxes.h"
#include "game/level/level_editor/player_layer.h"

typedef struct Player Player;
typedef struct Goals Goals;
typedef struct RigidBodies RigidBodies;

Player *create_player_from_player_layer(const PlayerLayer *player_layer,
                                        RigidBodies *rigid_bodies);
void destroy_player(Player * player);

int player_render(const Player * player,
                  const Camera *camera);
void player_update(Player * player,
                   float delta_time);
void player_touches_rect_sides(Player *player,
                               Rect object,
                               int sides[RECT_SIDE_N]);

int player_sound(Player *player,
                 Sound_samples *sound_samples);
void player_checkpoint(Player *player,
                       Vec2f checkpoint);

void player_move_left(Player *player);
void player_move_right(Player *player);
void player_stop(Player *player);
void player_jump(Player *player);
void player_die(Player *player);

void player_focus_camera(Player *player,
                         Camera *camera);
void player_die_from_lava(Player *player,
                          const Lava *lava);

bool player_overlaps_rect(const Player *player,
                          Rect rect);

Rect player_hitbox(const Player *player);

#endif  // PLAYER_H_
