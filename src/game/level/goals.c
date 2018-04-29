#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>

#include "goals.h"
#include "math/pi.h"
#include "math/triangle.h"
#include "system/error.h"
#include "system/lt.h"

#define GOAL_RADIUS 10.0f

static int goals_is_goal_hidden(const goals_t *goals, size_t i);

typedef enum cue_state_t {
    CUE_STATE_VIRGIN = 0,
    CUE_STATE_HIT_NOTHING,
    CUE_STATE_SEEN_NOTHING
} cue_state_t;

struct goals_t {
    lt_t *lt;
    point_t *points;
    rect_t *regions;
    color_t *colors;
    cue_state_t *cue_states;
    size_t goals_count;
    rect_t player_hitbox;
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

    goals->colors = PUSH_LT(lt, malloc(sizeof(color_t) * goals->goals_count), free);
    if (goals->colors == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    goals->cue_states = PUSH_LT(lt, malloc(sizeof(int) * goals->goals_count), free);
    if (goals->cue_states == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    char color[7];
    for (size_t i = 0; i < goals->goals_count; ++i) {
        if (fscanf(stream, "%f%f%f%f%f%f%6s",
                   &goals->points[i].x,
                   &goals->points[i].y,
                   &goals->regions[i].x,
                   &goals->regions[i].y,
                   &goals->regions[i].w,
                   &goals->regions[i].h,
                   color) < 0) {
            throw_error(ERROR_TYPE_LIBC);
            RETURN_LT(lt, NULL);
        }
        goals->colors[i] = color_from_hexstr(color);
        goals->cue_states[i] = CUE_STATE_VIRGIN;
    }

    goals->lt = lt;
    goals->angle = 0.0f;

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
    assert(goals);
    assert(renderer);
    assert(camera);

    const point_t position = vec_sum(
        goals->points[goal_index],
        vec(0.0f, sinf(goals->angle) * 10.0f));

    return camera_fill_triangle(
        camera,
        renderer,
        triangle_mat3x3_product(
            equilateral_triangle(),
            mat3x3_product2(
                trans_mat(position.x, position.y),
                rot_mat(PI * -0.5f + goals->angle),
                scale_mat(GOAL_RADIUS))),
        goals->colors[goal_index]);
}

int goals_render(const goals_t *goals,
                 SDL_Renderer *renderer,
                 const camera_t *camera)

{
    assert(goals);
    assert(renderer);
    assert(camera);

    for (size_t i = 0; i < goals->goals_count; ++i) {
        if (!goals_is_goal_hidden(goals, i)) {
            if (goals_render_core(goals, i, renderer, camera) < 0) {
                return -1;
            }
        }
    }

    return 0;
}

void goals_update(goals_t *goals,
                  float delta_time)
{
    assert(goals);
    assert(delta_time > 0.0f);
    goals->angle = fmodf(goals->angle + 2.0f * delta_time, 2.0f * PI);
}

void goals_hide(goals_t *goals,
                rect_t player_hitbox)
{
    goals->player_hitbox = player_hitbox;

}

int goals_sound(goals_t *goals,
                sound_medium_t *sound_medium)
{
    for (size_t i = 0; i < goals->goals_count; ++i) {
        switch (goals->cue_states[i]) {
        case CUE_STATE_HIT_NOTHING:
            sound_medium_play_sound(sound_medium, 0, goals->points[i], 0);
            goals->cue_states[i] = CUE_STATE_SEEN_NOTHING;
            break;

        default: {}
        }
    }

    return 0;
}

void goals_cue(goals_t *goals,
               SDL_Renderer *renderer,
               const camera_t *camera)
{
    for (size_t i = 0; i < goals->goals_count; ++i) {
        switch (goals->cue_states[i]) {
        case CUE_STATE_VIRGIN:
            if (goals_is_goal_hidden(goals, i) && camera_is_point_visible(camera, renderer, goals->points[i])) {
                goals->cue_states[i] = CUE_STATE_HIT_NOTHING;
            }

            break;

        case CUE_STATE_SEEN_NOTHING:
            if (!goals_is_goal_hidden(goals, i) && camera_is_point_visible(camera, renderer, goals->points[i])) {
                goals->cue_states[i] = CUE_STATE_VIRGIN;
            }
            break;

        default: {}
        }
    }
}

void goals_checkpoint(const goals_t *goals,
                      player_t *player)
{
    assert(goals);
    assert(player);

    for (size_t i = 0; i < goals->goals_count; ++i) {
        if (goals->cue_states[i] == CUE_STATE_HIT_NOTHING) {
            player_checkpoint(player, goals->points[i]);
        }
    }
}

/* Private Functions */

static int goals_is_goal_hidden(const goals_t *goals, size_t i)
{
    return rects_overlap(goals->regions[i], goals->player_hitbox);
}
