#include "system/stacktrace.h"

#include "player.h"
#include "regions.h"
#include "system/str.h"
#include "system/line_stream.h"
#include "system/log.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "game/level/level_editor/rect_layer.h"

#define ID_MAX_SIZE 36

enum RegionState {
    RS_PLAYER_OUTSIDE = 0,
    RS_PLAYER_INSIDE
};

struct Regions
{
    Lt *lt;
    size_t count;
    char *ids;
    Rect *rects;
    Color *colors;
    enum RegionState *states;
};

Regions *create_regions_from_line_stream(LineStream *line_stream)
{
    trace_assert(line_stream);

    Lt *lt = create_lt();

    Regions *regions = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(Regions)),
        free);
    if (regions == NULL) {
        RETURN_LT(lt, NULL);
    }
    regions->lt = lt;

    if(sscanf(
           line_stream_next(line_stream),
           "%zu",
           &regions->count) < 0) {
        log_fail("Could not read amount of script regions\n");
        RETURN_LT(lt, NULL);
    }

    regions->ids = PUSH_LT(
        lt,
        nth_calloc(regions->count * ID_MAX_SIZE, sizeof(char)),
        free);
    if (regions->ids == NULL) {
        RETURN_LT(lt, NULL);
    }

    regions->rects = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(Rect) * regions->count),
        free);
    if (regions->rects == NULL) {
        RETURN_LT(lt, NULL);
    }

    regions->colors = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(Color) * regions->count),
        free);
    if (regions->colors == NULL) {
        RETURN_LT(lt, NULL);
    }

    regions->states = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(enum RegionState) * regions->count),
        free);
    if (regions->states == NULL) {
        RETURN_LT(lt, NULL);
    }

    log_info("Amount of regions: %lu\n", regions->count);

    char color[7];

    for (size_t i = 0; i < regions->count; ++i) {
        if (sscanf(
                line_stream_next(line_stream),
                "%" STRINGIFY(ID_MAX_SIZE) "s%f%f%f%f%6s",
                regions->ids + ID_MAX_SIZE * i,
                &regions->rects[i].x,
                &regions->rects[i].y,
                &regions->rects[i].w,
                &regions->rects[i].h,
                color) < 0) {
            log_fail("Could not read size and color of %dth region\n");
            RETURN_LT(lt, NULL);
        }

        regions->colors[i] = hexstr(color);
        regions->states[i] = RS_PLAYER_OUTSIDE;
    }


    return regions;
}

Regions *create_regions_from_rect_layer(const RectLayer *rect_layer)
{
    trace_assert(rect_layer);

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
        nth_calloc(regions->count * ID_MAX_SIZE, sizeof(char)),
        free);
    if (regions->ids == NULL) {
        RETURN_LT(lt, NULL);
    }
    memcpy(regions->ids,
           rect_layer_ids(rect_layer),
           regions->count * ID_MAX_SIZE * sizeof(char));


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
