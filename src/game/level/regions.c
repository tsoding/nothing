#include <assert.h>

#include "player.h"
#include "regions.h"
#include "script.h"
#include "ebisp/gc.h"
#include "ebisp/interpreter.h"
#include "ebisp/parser.h"
#include "ebisp/scope.h"
#include "str.h"
#include "system/error.h"
#include "system/line_stream.h"
#include "system/lt.h"

enum RegionState {
    RS_PLAYER_INSIDE = 0,
    RS_PLAYER_OUTSIDE
};

struct Regions
{
    Lt *lt;
    size_t count;
    Rect *rects;
    Script **scripts;
    enum RegionState *states;
};

Regions *create_regions_from_line_stream(LineStream *line_stream, Level *level)
{
    assert(line_stream);

    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    Regions *regions = PUSH_LT(
        lt,
        malloc(sizeof(Regions)),
        free);
    if (regions == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    regions->lt = lt;

    if(sscanf(
           line_stream_next(line_stream),
           "%lu",
           &regions->count) < 0) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    regions->rects = PUSH_LT(
        lt,
        malloc(sizeof(Rect) * regions->count),
        free);
    if (regions->rects == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    regions->scripts = PUSH_LT(
        lt,
        malloc(sizeof(Script*) * regions->count),
        free);
    if (regions->scripts == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    regions->states = PUSH_LT(
        lt,
        malloc(sizeof(bool) * regions->count),
        free);
    if (regions->states == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    printf("Amount of regions: %lu\n", regions->count);

    for (size_t i = 0; i < regions->count; ++i) {
        if (sscanf(
                line_stream_next(line_stream),
                "%f%f%f%f",
                &regions->rects[i].x,
                &regions->rects[i].y,
                &regions->rects[i].w,
                &regions->rects[i].h) < 0) {
            throw_error(ERROR_TYPE_LIBC);
            RETURN_LT(lt, NULL);
        }

        regions->scripts[i] = PUSH_LT(
            lt,
            create_script_from_line_stream(line_stream, level),
            destroy_script);
        if (regions->scripts[i] == NULL) {
            RETURN_LT(lt, NULL);
        }

        /* TODO: Script doesn't provide its id on missing callback error */
        if (!script_has_scope_value(regions->scripts[i], "on-enter")) {
            fprintf(stderr, "Script does not provide on-enter callback\n");
            RETURN_LT(lt, NULL);
        }

        if (!script_has_scope_value(regions->scripts[i], "on-leave")) {
            fprintf(stderr, "Script does not provide on-leave callback\n");
            RETURN_LT(lt, NULL);
        }

        regions->states[i] = RS_PLAYER_OUTSIDE;
    }

    /* TODO(#456): create_regions_from_line_stream doesn't check if the scripts contain proper callbacks */

    return regions;
}

void destroy_regions(Regions *regions)
{
    assert(regions);
    RETURN_LT0(regions->lt);
}

void regions_player_enter(Regions *regions, Player *player)
{
    assert(regions);
    assert(player);

    for (size_t i = 0; i < regions->count; ++i) {
        if (regions->states[i] == RS_PLAYER_OUTSIDE &&
            player_overlaps_rect(player, regions->rects[i])) {
            regions->states[i] = RS_PLAYER_INSIDE;
            script_eval(regions->scripts[i], "(on-enter)");
        }
    }
}

void regions_player_leave(Regions *regions, Player *player)
{
    assert(regions);
    assert(player);

    for (size_t i = 0; i < regions->count; ++i) {
        if (regions->states[i] == RS_PLAYER_INSIDE &&
            !player_overlaps_rect(player, regions->rects[i])) {
            regions->states[i] = RS_PLAYER_OUTSIDE;
            script_eval(regions->scripts[i], "(on-leave)");
        }
    }
}
