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
    rect_t *regions;
    size_t goals_count;
    point_t player_position;
    float angle;
    float wave;
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

    goals->goals_count = 0;
    if (fscanf(stream, "%lu", &goals->goals_count) == EOF) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    goals->points = PUSH_LT(lt, malloc(sizeof(point_t) * goals->goals_count), free);
    if (goals->points == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    goals->regions = PUSH_LT(lt, malloc(sizeof(rect_t) * goals->goals_count), free);
    if (goals->regions == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    for (size_t i = 0; i < goals->goals_count; ++i) {
        if (fscanf(stream, "%f%f",
                   &goals->points[i].x,
                   &goals->points[i].y) < 0) {
            throw_error(ERROR_TYPE_LIBC);
            RETURN_LT(lt, NULL);
        }

        if (fscanf(stream, "%f%f%f%f\n",
                   &goals->regions[i].x,
                   &goals->regions[i].y,
                   &goals->regions[i].w,
                   &goals->regions[i].h) < 0) {
            throw_error(ERROR_TYPE_LIBC);
            RETURN_LT(lt, NULL);
        }
    }

    goals->lt = lt;
    goals->angle = 0.0f;
    goals->wave = 0.0f;

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

    for (size_t i = 0; i < goals->goals_count; ++i) {
        if (!rect_contains_point(goals->regions[i], goals->player_position)) {
            /* Core */

            if (SDL_SetRenderDrawColor(renderer, 255, 255, 50, 255) < 0) {
                throw_error(ERROR_TYPE_SDL2);
                return -1;
            }

            const rect_t hitbox = {
                .x = goals->points[i].x - GOALS_WIDTH / 2.0f,
                .y = goals->points[i].y - GOALS_HEIGHT / 2.0f + sinf(goals->angle) * 10.0f,
                .w = GOALS_WIDTH,
                .h = GOALS_HEIGHT
            };

            if (camera_fill_rect(camera, renderer, &hitbox) < 0) {
                return -1;
            }

            /* Wave */

            const Uint8 alpha = (Uint8) (roundf(255.0f * (1.0f - fminf(goals->wave, 1.0f))) * 0.5f);

            if (SDL_SetRenderDrawColor(renderer, 255, 255, 50, alpha) < 0) {
                throw_error(ERROR_TYPE_SDL2);
                return -1;
            }

            const float wave_scale_factor = fminf(goals->wave, 1.0f) * 10.0f;

            const rect_t wavebox = {
                .x = goals->points[i].x - GOALS_WIDTH * wave_scale_factor / 2.0f,
                .y = goals->points[i].y - GOALS_HEIGHT * wave_scale_factor / 2.0f + sinf(goals->angle) * 10.0f,
                .w = GOALS_WIDTH * wave_scale_factor,
                .h = GOALS_HEIGHT * wave_scale_factor
            };

            if (camera_draw_rect(camera, renderer, &wavebox) < 0) {
                return -1;
            }
        }
    }

    return 0;
}

void goals_update(goals_t *goals,
                  Uint32 delta_time)
{
    assert(goals);
    assert(delta_time > 0);

    float d = (float) delta_time / 1000.0f;

    goals->angle = fmodf(goals->angle + 2.0f * d, 2.0f * PI);
    goals->wave = fmodf(goals->wave + 0.35f * d, 2.0f);
}

void goals_hide(goals_t *goals,
                vec_t player_position)
{
    goals->player_position = player_position;
}
