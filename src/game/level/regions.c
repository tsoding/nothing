#include <assert.h>

#include "player.h"
#include "regions.h"
#include "script.h"
#include "script/gc.h"
#include "script/interpreter.h"
#include "script/parser.h"
#include "script/scope.h"
#include "str.h"
#include "system/error.h"
#include "system/line_stream.h"
#include "system/lt.h"

struct Regions
{
    Lt *lt;
    size_t count;
    Rect *rects;
    Script **scripts;
};

Regions *create_regions_from_line_stream(LineStream *line_stream)
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
            create_script_from_line_stream(line_stream),
            destroy_script);
        if (regions->scripts[i] == NULL) {
            RETURN_LT(lt, NULL);
        }
    }

    /* TODO: create_regions_from_line_stream doesn't check if the scripts contain proper callbacks */

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
    /* TODO(#396): regions_player_enter is not implemented */
}

void regions_player_leave(Regions *regions, Player *player)
{
    assert(regions);
    assert(player);
    /* TODO(#397): regions_player_leave is not implemented */
}
