#include <assert.h>
#include <math.h>

#include <SDL2/SDL.h>

#include "./lt.h"
#include "./goals.h"
#include "./error.h"

#define PI 3.14159265359f
#define GOALS_WIDTH 10.0f
#define GOALS_HEIGHT 10.0f

struct goals_t {
    lt_t *lt;
    float x, y;
    float angle;
};

goals_t *create_goals(float x, float y)
{
    lt_t *lt = create_lt();

    if (lt == NULL) {
        return NULL;
    }

    goals_t *goals = PUSH_LT(lt, malloc(sizeof(goals_t)), free);
    if (goals == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    goals->lt = lt;
    goals->x = x;
    goals->y = y;
    goals->angle = 0.0f;

    return goals;
}

void destroy_goals(goals_t *goals)
{
    assert(goals);
    RETURN_LT0(goals->lt);
}

rect_t goals_hitbox(const goals_t *goals)
{
    const rect_t hitbox = {
        .x = goals->x - GOALS_WIDTH / 2.0f,
        .y = goals->y - GOALS_HEIGHT + sinf(goals->angle) * 10.0f,
        .w = GOALS_WIDTH,
        .h = GOALS_HEIGHT
    };

    return hitbox;
}

int goals_render(const goals_t *goals,
                SDL_Renderer *renderer,
                const camera_t *camera)

{
    assert(goals);
    assert(renderer);
    assert(camera);

    if (SDL_SetRenderDrawColor(renderer, 255, 255, 96, 255) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    rect_t hitbox = goals_hitbox(goals);

    return camera_fill_rect(camera, renderer, &hitbox);
}

void goals_update(goals_t *goals, Uint32 delta_time)
{
    assert(goals);
    assert(delta_time > 0);

    float d = (float) delta_time / 1000.0f;

    goals->angle = fmodf(goals->angle + 2.0f * d, 2.0f * PI);
}
