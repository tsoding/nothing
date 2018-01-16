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
    point_t *points;
    size_t points_count;
    float angle;
};


goals_t *create_goals_from_stream(FILE *stream)
{
    assert(stream);

    lt_t *const lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    goals_t *const goals = PUSH_LT(lt, malloc(sizeof(goals_t)), free);
    if (goals == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    goals->points_count = 0;
    if (fscanf(stream, "%lu", &goals->points_count) == EOF) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    goals->points = PUSH_LT(lt, malloc(sizeof(point_t) * goals->points_count), free);
    if (goals->points == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    for (size_t i = 0; i < goals->points_count; ++i) {
        if (fscanf(stream, "%f%f\n",
                   &goals->points[i].x,
                   &goals->points[i].y) < 0) {
            throw_error(ERROR_TYPE_LIBC);
            RETURN_LT(lt, NULL);
        }
    }

    goals->lt = lt;

    return goals;
}

void destroy_goals(goals_t *goals)
{
    assert(goals);
    RETURN_LT0(goals->lt);
}

int goals_render(const goals_t *goals,
                SDL_Renderer *renderer,
                const camera_t *camera)

{
    assert(goals);
    assert(renderer);
    assert(camera);

    if (SDL_SetRenderDrawColor(renderer, 255, 255, 50, 255) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    for (size_t i = 0; i < goals->points_count; ++i) {
        const rect_t hitbox = {
            .x = goals->points[i].x - GOALS_WIDTH / 2.0f,
            .y = goals->points[i].y - GOALS_HEIGHT + sinf(goals->angle) * 10.0f,
            .w = GOALS_WIDTH,
            .h = GOALS_HEIGHT
        };

        if (camera_fill_rect(camera, renderer, &hitbox) < 0) {
            return -1;
        }
    }

    return 0;
}

void goals_update(goals_t *goals, Uint32 delta_time)
{
    assert(goals);
    assert(delta_time > 0);

    float d = (float) delta_time / 1000.0f;

    goals->angle = fmodf(goals->angle + 2.0f * d, 2.0f * PI);
}
