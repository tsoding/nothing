#include <stdio.h>
#include <math.h>

#include <SDL.h>

#include "game/level/level_editor/point_layer.h"
#include "goals.h"
#include "math/pi.h"
#include "math/triangle.h"
#include "system/log.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/stacktrace.h"
#include "system/str.h"

#define GOAL_RADIUS 10.0f

static int goals_is_goal_hidden(const Goals *goals, size_t i);

typedef enum Cue_state {
    CUE_STATE_VIRGIN = 0,
    CUE_STATE_HIT_NOTHING,
    CUE_STATE_SEEN_NOTHING
} Cue_state;

struct Goals {
    Lt *lt;
    char **ids;
    Vec2f *positions;
    Color *colors;
    Cue_state *cue_states;
    bool *visible;
    size_t count;
    float angle;
};

Goals *create_goals_from_point_layer(const PointLayer *point_layer)
{
    trace_assert(point_layer);

    Lt *lt = create_lt();

    Goals *const goals = PUSH_LT(lt, nth_calloc(1, sizeof(Goals)), free);
    if (goals == NULL) {
        RETURN_LT(lt, NULL);
    }

    goals->count = point_layer_count(point_layer);

    goals->ids = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(char*) * goals->count),
        free);
    if (goals->ids == NULL) {
        RETURN_LT(lt, NULL);
    }
    for (size_t i = 0; i < goals->count; ++i) {
        goals->ids[i] = PUSH_LT(lt, nth_calloc(1, sizeof(char) * ENTITY_MAX_ID_SIZE), free);
        if (goals->ids[i] == NULL) {
            RETURN_LT(lt, NULL);
        }
    }

    goals->positions = PUSH_LT(lt, nth_calloc(1, sizeof(Vec2f) * goals->count), free);
    if (goals->positions == NULL) {
        RETURN_LT(lt, NULL);
    }

    goals->colors = PUSH_LT(lt, nth_calloc(1, sizeof(Color) * goals->count), free);
    if (goals->colors == NULL) {
        RETURN_LT(lt, NULL);
    }

    goals->cue_states = PUSH_LT(lt, nth_calloc(1, sizeof(int) * goals->count), free);
    if (goals->cue_states == NULL) {
        RETURN_LT(lt, NULL);
    }

    goals->visible = PUSH_LT(lt, nth_calloc(1, sizeof(bool) * goals->count), free);
    if (goals->visible == NULL) {
        RETURN_LT(lt, NULL);
    }

    const Vec2f *positions = point_layer_positions(point_layer);
    const Color *colors = point_layer_colors(point_layer);
    const char *ids = point_layer_ids(point_layer);

    // TODO(#835): we could use memcpy in create_goals_from_point_layer
    for (size_t i = 0; i < goals->count; ++i) {
        goals->positions[i] = positions[i];
        goals->colors[i] = colors[i];
        memcpy(goals->ids[i], ids + ID_MAX_SIZE * i, ID_MAX_SIZE);
        goals->cue_states[i] = CUE_STATE_VIRGIN;
        goals->visible[i] = true;
    }

    goals->lt = lt;
    goals->angle = 0.0f;

    return goals;
}

void destroy_goals(Goals *goals)
{
    trace_assert(goals);
    RETURN_LT0(goals->lt);
}

static int goals_render_core(const Goals *goals,
                             size_t goal_index,
                             const Camera *camera)
{
    trace_assert(goals);
    trace_assert(camera);

    const Vec2f position = vec_sum(
        goals->positions[goal_index],
        vec(0.0f, sinf(goals->angle) * 10.0f));

    if (camera_fill_triangle(
            camera,
            triangle_mat3x3_product(
                equilateral_triangle(),
                mat3x3_product2(
                    trans_mat(position.x, position.y),
                    rot_mat(PI * -0.5f + goals->angle),
                    scale_mat(GOAL_RADIUS))),
            goals->colors[goal_index]) < 0) {
        return -1;
    }

    if (camera_render_debug_text(
            camera,
            goals->ids[goal_index],
            position) < 0) {
        return -1;
    }

    return 0;
}

int goals_render(const Goals *goals,
                 const Camera *camera)
{
    trace_assert(goals);
    trace_assert(camera);

    for (size_t i = 0; i < goals->count; ++i) {
        if (!goals_is_goal_hidden(goals, i)) {
            if (goals_render_core(goals, i, camera) < 0) {
                return -1;
            }
        }
    }

    return 0;
}

void goals_update(Goals *goals,
                  float delta_time)
{
    trace_assert(goals);
    trace_assert(delta_time > 0.0f);
    goals->angle = fmodf(goals->angle + 2.0f * delta_time, 2.0f * PI);
}

int goals_sound(Goals *goals,
                Sound_samples *sound_samples)
{
    for (size_t i = 0; i < goals->count; ++i) {
        switch (goals->cue_states[i]) {
        case CUE_STATE_HIT_NOTHING:
            sound_samples_play_sound(sound_samples, 0);
            goals->cue_states[i] = CUE_STATE_SEEN_NOTHING;
            break;

        default: {}
        }
    }

    return 0;
}

void goals_cue(Goals *goals,
               const Camera *camera)
{
    for (size_t i = 0; i < goals->count; ++i) {
        switch (goals->cue_states[i]) {
        case CUE_STATE_VIRGIN:
            if (goals_is_goal_hidden(goals, i) && camera_is_point_visible(camera, goals->positions[i])) {
                goals->cue_states[i] = CUE_STATE_HIT_NOTHING;
            }

            break;

        case CUE_STATE_SEEN_NOTHING:
            if (!goals_is_goal_hidden(goals, i) && camera_is_point_visible(camera, goals->positions[i])) {
                goals->cue_states[i] = CUE_STATE_VIRGIN;
            }
            break;

        default: {}
        }
    }
}

void goals_checkpoint(const Goals *goals,
                      Player *player)
{
    trace_assert(goals);
    trace_assert(player);

    for (size_t i = 0; i < goals->count; ++i) {
        if (goals->cue_states[i] == CUE_STATE_HIT_NOTHING) {
            player_checkpoint(player, goals->positions[i]);
        }
    }
}

/* Private Functions */

static int goals_is_goal_hidden(const Goals *goals, size_t i)
{
    return !goals->visible[i];
}

void goals_hide(Goals *goals, char goal_id[ENTITY_MAX_ID_SIZE])
{
    trace_assert(goals);
    trace_assert(goal_id);

    for (size_t i = 0; i < goals->count; ++i) {
        if (strncmp(goal_id, goals->ids[i], ENTITY_MAX_ID_SIZE) == 0) {
            goals->visible[i] = false;
        }
    }
}

void goals_show(Goals *goals, char goal_id[ENTITY_MAX_ID_SIZE])
{
    trace_assert(goals);
    trace_assert(goal_id);
    for (size_t i = 0; i < goals->count; ++i) {
        if (strncmp(goal_id, goals->ids[i], ENTITY_MAX_ID_SIZE) == 0) {
            goals->visible[i] = true;
        }
    }
}
