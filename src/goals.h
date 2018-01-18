#ifndef GOALS_H_
#define GOALS_H_

#include "./camera.h"

typedef struct goals_t goals_t;
typedef struct SDL_Renderer SDL_Renderer;

// TODO(#64): implement goal hiding mechanism
// TODO(#65): add some glow to the goal to make it stand out

goals_t *create_goals_from_stream(FILE *stream);
void destroy_goals(goals_t *goals);

rect_t goals_hitbox(const goals_t *goals);

int goals_render(const goals_t *goals,
                SDL_Renderer *renderer,
                const camera_t *camera);
void goals_update(goals_t *goals,
                  Uint32 delta_time);
void goals_hide(goals_t *goals,
                vec_t player_position);

#endif  // GOALS_H_
