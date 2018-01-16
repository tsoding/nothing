#include <assert.h>
#include <math.h>

#include <SDL2/SDL.h>

#include "./lt.h"
#include "./goal.h"
#include "./error.h"

#define PI 3.14159265359f
#define GOAL_WIDTH 10.0f
#define GOAL_HEIGHT 10.0f

struct goal_t {
    lt_t *lt;
    float x, y;
    float angle;
};

goal_t *create_goal(float x, float y)
{
    lt_t *lt = create_lt();

    if (lt == NULL) {
        return NULL;
    }

    goal_t *goal = PUSH_LT(lt, malloc(sizeof(goal_t)), free);
    if (goal == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    goal->lt = lt;
    goal->x = x;
    goal->y = y;
    goal->angle = 0.0f;

    return goal;
}

void destroy_goal(goal_t *goal)
{
    assert(goal);
    RETURN_LT0(goal->lt);
}

rect_t goal_hitbox(const goal_t *goal)
{
    const rect_t hitbox = {
        .x = goal->x - GOAL_WIDTH / 2.0f,
        .y = goal->y - GOAL_HEIGHT + sinf(goal->angle) * 10.0f,
        .w = GOAL_WIDTH,
        .h = GOAL_HEIGHT
    };

    return hitbox;
}

int goal_render(const goal_t *goal,
                SDL_Renderer *renderer,
                const camera_t *camera)

{
    assert(goal);
    assert(renderer);
    assert(camera);

    if (SDL_SetRenderDrawColor(renderer, 255, 255, 96, 255) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    rect_t hitbox = goal_hitbox(goal);

    return camera_fill_rect(camera, renderer, &hitbox);
}

void goal_update(goal_t *goal, Uint32 delta_time)
{
    assert(goal);
    assert(delta_time > 0);

    float d = (float) delta_time / 1000.0f;

    goal->angle = fmodf(goal->angle + 2.0f * d, 2.0f * PI);
}
