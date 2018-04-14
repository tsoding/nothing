#ifndef GOALS_H_
#define GOALS_H_

#include <SDL2/SDL.h>
#include "./camera.h"
#include "./sound_medium.h"

typedef struct goals_t goals_t;

goals_t *create_goals_from_stream(FILE *stream);
void destroy_goals(goals_t *goals);

rect_t goals_hitbox(const goals_t *goals);

int goals_render(const goals_t *goals,
                 SDL_Renderer *renderer,
                 const camera_t *camera);
int goals_sound(goals_t *goals,
                sound_medium_t *sound_medium);
void goals_update(goals_t *goals,
                  Uint32 delta_time);
void goals_hide(goals_t *goals,
                rect_t player_hitbox);

#endif  // GOALS_H_
