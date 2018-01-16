#ifndef GOAL_H_
#define GOAL_H_

#include "./camera.h"

typedef struct goal_t goal_t;
typedef struct SDL_Renderer SDL_Renderer;

goal_t *create_goal(float x, float y);
void destroy_goal(goal_t *goal);

rect_t goal_hitbox(const goal_t *goal);

int goal_render(const goal_t *goal,
                SDL_Renderer *renderer,
                const camera_t *camera);
void goal_update(goal_t *goal, Uint32 delta_time);

#endif  // GOAL_H_
