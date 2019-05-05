#ifndef GOALS_H_
#define GOALS_H_

#include <SDL2/SDL.h>

#include "game/camera.h"
#include "game/level/player.h"
#include "game/sound_samples.h"
#include "ebisp/expr.h"

typedef struct Goals Goals;
typedef struct LineStream LineStream;
typedef struct PointLayer PointLayer;

Goals *create_goals_from_line_stream(LineStream *line_stream);
Goals *create_goals_from_point_layer(const PointLayer *point_layer);
void destroy_goals(Goals *goals);

Rect goals_hitbox(const Goals *goals);

int goals_render(const Goals *goals,
                 Camera *camera);
int goals_sound(Goals *goals,
                Sound_samples *sound_samples);
void goals_update(Goals *goals,
                  float delta_time);
void goals_hide_from_player(Goals *goals,
                            Rect player_hitbox);
void goals_checkpoint(const Goals *goals,
                      Player *player);
void goals_cue(Goals *goals,
               const Camera *camera);

struct EvalResult goals_send(Goals *goals, Gc *gc, struct Scope *scope, struct Expr path);

#endif  // GOALS_H_
