#include <assert.h>
#include <math.h>

#include <SDL2/SDL.h>

#include "./lt.h"
#include "./goals.h"
#include "./error.h"
#include "./pi.h"
#include "./triangle.h"

#define GOAL_RADIUS 10.0f

struct goals_t {
    lt_t *lt;
    point_t *points;
    rect_t *regions;
    size_t goals_count;
    rect_t player_hitbox;
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

static int goals_render_core(const goals_t *goals,
                             size_t goal_index,
                             SDL_Renderer *renderer,
                             const camera_t *camera)
{
    if (SDL_SetRenderDrawColor(renderer, 255, 255, 50, 255) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }


    const point_t position =
        vec_sum(
            goals->points[goal_index],
            vec(0.0f, sinf(goals->angle) * 10.0f));

    const triangle_t core = equilateral_triangle(position, GOAL_RADIUS, PI * -0.5f + goals->angle);

    if (camera_fill_triangle(
            camera,
            renderer,
            core.p1,
            core.p2,
            core.p3) < 0) {
        return -1;
    }

    return 0;
}

static int goals_render_wave(const goals_t *goals,
                             size_t goal_index,
                             SDL_Renderer *renderer,
                             const camera_t *camera)
{
    const Uint8 alpha = (Uint8) (roundf(255.0f * (1.0f - fminf(goals->wave, 1.0f))) * 0.5f);

    if (SDL_SetRenderDrawColor(renderer, 255, 255, 50, alpha) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    const point_t position =
        vec_sum(
            goals->points[goal_index],
            vec(0.0f, sinf(goals->angle) * 10.0f));

    const float wave_scale_factor = fminf(goals->wave, 1.0f) * 10.0f;
    const triangle_t core = equilateral_triangle(position, GOAL_RADIUS * wave_scale_factor, PI * -0.5f + goals->angle);

    if (camera_draw_triangle(
            camera,
            renderer,
            core.p1,
            core.p2,
            core.p3) < 0) {
        return -1;
    }

    return 0;
}

int goals_render(const goals_t *goals,
                 SDL_Renderer *renderer,
                 const camera_t *camera)

{
    assert(goals);
    assert(renderer);
    assert(camera);

    for (size_t i = 0; i < goals->goals_count; ++i) {
        if (!rects_overlap(goals->regions[i], goals->player_hitbox)) {
            if (goals_render_core(goals, i, renderer, camera) < 0) {
                return -1;
            }

            if (goals_render_wave(goals, i, renderer, camera) < 0) {
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
    goals->wave = fmodf(goals->wave + 0.35f * d, 1.5f);
}

void goals_hide(goals_t *goals,
                rect_t player_hitbox)
{
    goals->player_hitbox = player_hitbox;

}
