#include <SDL2/SDL.h>
#include "system/stacktrace.h"
#include <math.h>

#include "goals.h"
#include "math/pi.h"
#include "math/triangle.h"
#include "str.h"
#include "system/line_stream.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/log.h"
#ifdef WINDOWS_OS
#include <stdio.h> // for EOF macro
#endif

#define GOAL_RADIUS 10.0f
#define GOAL_MAX_ID_SIZE 36

static int goals_is_goal_hidden(const Goals *goals, size_t i);

typedef enum Cue_state {
    CUE_STATE_VIRGIN = 0,
    CUE_STATE_HIT_NOTHING,
    CUE_STATE_SEEN_NOTHING
} Cue_state;

struct Goals {
    Lt *lt;
    char **ids;
    Point *points;
    Color *colors;
    Cue_state *cue_states;
    bool *visible;
    size_t count;
    Rect player_hitbox;
    float angle;
};

Goals *create_goals_from_line_stream(LineStream *line_stream)
{
    trace_assert(line_stream);

    Lt *const lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    Goals *const goals = PUSH_LT(lt, nth_alloc(sizeof(Goals)), free);
    if (goals == NULL) {
        RETURN_LT(lt, NULL);
    }

    goals->count = 0;
    if (sscanf(
            line_stream_next(line_stream),
            #ifdef X64_BUILD
            "%llu",
#else
            "%lu",
#endif
            &goals->count) == EOF) {
        log_fail("Could not read amount of goals\n");
        RETURN_LT(lt, NULL);
    }

    goals->ids = PUSH_LT(
        lt,
        nth_alloc(sizeof(char*) * goals->count),
        free);
    if (goals->ids == NULL) {
        RETURN_LT(lt, NULL);
    }
    for (size_t i = 0; i < goals->count; ++i) {
        goals->ids[i] = PUSH_LT(lt, nth_alloc(sizeof(char) * GOAL_MAX_ID_SIZE), free);
        if (goals->ids[i] == NULL) {
            RETURN_LT(lt, NULL);
        }
    }

    goals->points = PUSH_LT(lt, nth_alloc(sizeof(Point) * goals->count), free);
    if (goals->points == NULL) {
        RETURN_LT(lt, NULL);
    }

    goals->colors = PUSH_LT(lt, nth_alloc(sizeof(Color) * goals->count), free);
    if (goals->colors == NULL) {
        RETURN_LT(lt, NULL);
    }

    goals->cue_states = PUSH_LT(lt, nth_alloc(sizeof(int) * goals->count), free);
    if (goals->cue_states == NULL) {
        RETURN_LT(lt, NULL);
    }

    goals->visible = PUSH_LT(lt, nth_alloc(sizeof(bool) * goals->count), free);
    if (goals->visible == NULL) {
        RETURN_LT(lt, NULL);
    }

    char color[7];
    for (size_t i = 0; i < goals->count; ++i) {
        if (sscanf(
                line_stream_next(line_stream),
                "%" STRINGIFY(GOAL_MAX_ID_SIZE) "s%f%f%6s",
                goals->ids[i],
                &goals->points[i].x,
                &goals->points[i].y,
                color) < 0) {
            log_fail("Could not read %dth goal\n", i);
            RETURN_LT(lt, NULL);
        }
        goals->colors[i] = hexstr(color);
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
                             Camera *camera)
{
    trace_assert(goals);
    trace_assert(camera);

    const Point position = vec_sum(
        goals->points[goal_index],
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
                 Camera *camera)
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

void goals_hide_from_player(Goals *goals,
                            Rect player_hitbox)
{
    goals->player_hitbox = player_hitbox;

}

int goals_sound(Goals *goals,
                Sound_samples *sound_samples)
{
    for (size_t i = 0; i < goals->count; ++i) {
        switch (goals->cue_states[i]) {
        case CUE_STATE_HIT_NOTHING:
            sound_samples_play_sound(sound_samples, 0, 0);
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
            if (goals_is_goal_hidden(goals, i) && camera_is_point_visible(camera, goals->points[i])) {
                goals->cue_states[i] = CUE_STATE_HIT_NOTHING;
            }

            break;

        case CUE_STATE_SEEN_NOTHING:
            if (!goals_is_goal_hidden(goals, i) && camera_is_point_visible(camera, goals->points[i])) {
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
            player_checkpoint(player, goals->points[i]);
        }
    }
}

void goals_hide(Goals *goals, const char *id)
{
    trace_assert(goals);
    trace_assert(id);

    for (size_t i = 0; i < goals->count; ++i) {
        if (strcmp(id, goals->ids[i]) == 0) {
            goals->visible[i] = false;
            return;
        }
    }
}

void goals_show(Goals *goals, const char *id)
{
    trace_assert(goals);
    trace_assert(id);

    for (size_t i = 0; i < goals->count; ++i) {
        if (strcmp(id, goals->ids[i]) == 0) {
            goals->visible[i] = true;
            return;
        }
    }
}

/* Private Functions */

static int goals_is_goal_hidden(const Goals *goals, size_t i)
{
    return !goals->visible[i];
}
