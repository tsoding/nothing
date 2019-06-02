#include "system/stacktrace.h"

#include "ebisp/gc.h"
#include "ebisp/interpreter.h"
#include "ebisp/parser.h"
#include "ebisp/scope.h"
#include "player.h"
#include "regions.h"
#include "script.h"
#include "system/str.h"
#include "system/line_stream.h"
#include "system/log.h"
#include "system/lt.h"
#include "system/nth_alloc.h"

enum RegionState {
    RS_PLAYER_INSIDE = 0,
    RS_PLAYER_OUTSIDE
};

struct Regions
{
    Lt *lt;
    size_t count;
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
           "%lu",
           &regions->count) < 0) {
        log_fail("Could not read amount of script regions\n");
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
                "%f%f%f%f%6s",
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

int regions_render(Regions *regions, Camera *camera)
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
