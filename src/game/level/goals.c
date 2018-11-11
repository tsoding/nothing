#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>

#include "goals.h"
#include "math/pi.h"
#include "math/triangle.h"
#include "str.h"
#include "system/error.h"
#include "system/line_stream.h"
#include "system/lt.h"
#include "system/nth_alloc.h"

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
    /* TODO(#493): replace Goals.regions with the Regions entity */
    Rect *regions;
    Color *colors;
    /* TODO(#494): it is not clear how to maintain Cue_state from the scripting language */
    Cue_state *cue_states;
    bool *visible;
    size_t count;
    Rect player_hitbox;
    float angle;
};

Goals *create_goals_from_line_stream(LineStream *line_stream)
{
    assert(line_stream);

    Lt *const lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    Goals *const goals = PUSH_LT(lt, nth_alloc(sizeof(Goals)), free);
    if (goals == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    goals->count = 0;
    if (sscanf(
            line_stream_next(line_stream),
            "%lu",
            &goals->count) == EOF) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    goals->ids = PUSH_LT(
        lt,
        nth_alloc(sizeof(char*) * goals->count),
        free);
    if (goals->ids == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    for (size_t i = 0; i < goals->count; ++i) {
        goals->ids[i] = PUSH_LT(lt, nth_alloc(sizeof(char) * GOAL_MAX_ID_SIZE), free);
        if (goals->ids[i] == NULL) {
            throw_error(ERROR_TYPE_LIBC);
            RETURN_LT(lt, NULL);
        }
    }

    goals->points = PUSH_LT(lt, nth_alloc(sizeof(Point) * goals->count), free);
    if (goals->points == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    goals->regions = PUSH_LT(lt, nth_alloc(sizeof(Rect) * goals->count), free);
    if (goals->regions == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    goals->colors = PUSH_LT(lt, nth_alloc(sizeof(Color) * goals->count), free);
    if (goals->colors == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    goals->cue_states = PUSH_LT(lt, nth_alloc(sizeof(int) * goals->count), free);
    if (goals->cue_states == NULL) {
        throw_error(ERROR_TYPE_LIBC);
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
                "%" STRINGIFY(GOAL_MAX_ID_SIZE) "s%f%f%f%f%f%f%6s",
                goals->ids[i],
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
        goals->visible[i] = true;
    }

    goals->lt = lt;
    goals->angle = 0.0f;

    return goals;
}

void destroy_goals(Goals *goals)
{
    assert(goals);
    RETURN_LT0(goals->lt);
}

static int goals_render_core(const Goals *goals,
                             size_t goal_index,
                             Camera *camera)
{
    assert(goals);
    assert(camera);

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

/* TODO(#448): goals do not render their ids in debug mode */
int goals_render(const Goals *goals,
                 Camera *camera)
{
    assert(goals);
    assert(camera);

    for (size_t i = 0; i < goals->count; ++i) {
        if (!goals_is_goal_hidden(goals, i)) {
            if (goals_render_core(goals, i, camera) < 0) {
                return -1;
            }
        }

        if (camera_render_debug_rect(
                camera,
                goals->regions[i],
                goals->colors[i]) < 0) {
            return -1;
        }
    }

    return 0;
}

void goals_update(Goals *goals,
                  float delta_time)
{
    assert(goals);
    assert(delta_time > 0.0f);
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
    assert(goals);
    assert(player);

    for (size_t i = 0; i < goals->count; ++i) {
        if (goals->cue_states[i] == CUE_STATE_HIT_NOTHING) {
            player_checkpoint(player, goals->points[i]);
        }
    }
}

void goals_hide(Goals *goals, const char *id)
{
    assert(goals);
    assert(id);

    for (size_t i = 0; i < goals->count; ++i) {
        if (strcmp(id, goals->ids[i]) == 0) {
            goals->visible[i] = false;
            return;
        }
    }
}

void goals_show(Goals *goals, const char *id)
{
    assert(goals);
    assert(id);

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
