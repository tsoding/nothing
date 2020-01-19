#include "system/stacktrace.h"

#include "config.h"
#include "player.h"
#include "regions.h"
#include "system/str.h"
#include "system/log.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "game/level/level_editor/rect_layer.h"
#include "game/level/labels.h"
#include "game/level/goals.h"

enum RegionState {
    RS_PLAYER_OUTSIDE = 0,
    RS_PLAYER_INSIDE
};

struct Regions {
    Lt *lt;
    size_t count;
    char *ids;
    Rect *rects;
    Color *colors;
    enum RegionState *states;
    Action *actions;

    Labels *labels;
    Goals *goals;
};

Regions *create_regions_from_rect_layer(const RectLayer *rect_layer,
                                        Labels *labels,
                                        Goals *goals)
{
    trace_assert(rect_layer);
    trace_assert(labels);

    Lt *lt = create_lt();

    Regions *regions = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(Regions)),
        free);
    if (regions == NULL) {
        RETURN_LT(lt, NULL);
    }
    regions->lt = lt;

    regions->count = rect_layer_count(rect_layer);

    regions->ids = PUSH_LT(
        lt,
        nth_calloc(regions->count * ENTITY_MAX_ID_SIZE, sizeof(char)),
        free);
    if (regions->ids == NULL) {
        RETURN_LT(lt, NULL);
    }
    memcpy(regions->ids,
           rect_layer_ids(rect_layer),
           regions->count * ENTITY_MAX_ID_SIZE * sizeof(char));


    regions->rects = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(Rect) * regions->count),
        free);
    if (regions->rects == NULL) {
        RETURN_LT(lt, NULL);
    }
    memcpy(regions->rects,
           rect_layer_rects(rect_layer),
           regions->count * sizeof(Rect));


    regions->colors = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(Color) * regions->count),
        free);
    if (regions->colors == NULL) {
        RETURN_LT(lt, NULL);
    }
    memcpy(regions->colors,
           rect_layer_colors(rect_layer),
           regions->count * sizeof(Color));

    regions->states = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(enum RegionState) * regions->count),
        free);
    if (regions->states == NULL) {
        RETURN_LT(lt, NULL);
    }

    regions->actions = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(Action) * regions->count),
        free);
    if (regions->actions == NULL) {
        RETURN_LT(lt, NULL);
    }
    memcpy(regions->actions,
           rect_layer_actions(rect_layer),
           regions->count * sizeof(Action));

    // TODO(#1108): impossible to change the region action from the Level Editor


    regions->labels = labels;
    regions->goals = goals;

    return regions;
}

void destroy_regions(Regions *regions)
{
    trace_assert(regions);
    RETURN_LT0(regions->lt);
}

void regions_player_enter(Regions *regions, Player *player)
{
    trace_assert(regions);
    trace_assert(player);

    for (size_t i = 0; i < regions->count; ++i) {
        if (regions->states[i] == RS_PLAYER_OUTSIDE &&
            player_overlaps_rect(player, regions->rects[i])) {
            regions->states[i] = RS_PLAYER_INSIDE;

            switch (regions->actions[i].type) {
            case ACTION_HIDE_LABEL: {
                labels_hide(regions->labels, regions->actions[i].entity_id);
            } break;

            case ACTION_TOGGLE_GOAL: {
                goals_hide(regions->goals, regions->actions[i].entity_id);
            } break;

            default: {}
            }
        }
    }
}

void regions_player_leave(Regions *regions, Player *player)
{
    trace_assert(regions);
    trace_assert(player);

    for (size_t i = 0; i < regions->count; ++i) {
        if (regions->states[i] == RS_PLAYER_INSIDE &&
            !player_overlaps_rect(player, regions->rects[i])) {
            regions->states[i] = RS_PLAYER_OUTSIDE;

            switch (regions->actions[i].type) {
            case ACTION_TOGGLE_GOAL: {
                goals_show(regions->goals, regions->actions[i].entity_id);
            } break;

            default: {}
            }
        }
    }
}

int regions_render(Regions *regions, const Camera *camera)
{
    trace_assert(regions);
    trace_assert(camera);

    for (size_t i = 0; i < regions->count; ++i) {
        if (camera_render_debug_rect(
                camera,
                regions->rects[i],
                regions->colors[i]) < 0) {
            return -1;
        }
    }

    return 0;
}
