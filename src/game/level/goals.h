#ifndef GOALS_H_
#define GOALS_H_

#include <SDL.h>

#include "game/camera.h"
#include "game/level/player.h"
#include "game/sound_samples.h"
#include "game/level/level_editor/point_layer.h"
#include "config.h"

typedef struct Goals Goals;

Goals *create_goals_from_point_layer(const PointLayer *point_layer);
void destroy_goals(Goals *goals);

Rect goals_hitbox(const Goals *goals);

int goals_render(const Goals *goals,
                 const Camera *camera);
int goals_sound(Goals *goals,
                Sound_samples *sound_samples);
void goals_update(Goals *goals,
                  float delta_time);
void goals_checkpoint(const Goals *goals,
                      Player *player);
void goals_cue(Goals *goals,
               const Camera *camera);

void goals_hide(Goals *goals, char goal_id[ENTITY_MAX_ID_SIZE]);
void goals_show(Goals *goals, char goal_id[ENTITY_MAX_ID_SIZE]);

#endif  // GOALS_H_
