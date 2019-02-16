#include <SDL2/SDL.h>
#include "system/stacktrace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "platforms.h"
#include "system/lt.h"
#include "system/lt/lt_adapters.h"
#include "system/line_stream.h"
#include "system/nth_alloc.h"
#include "system/log.h"

struct Platforms {
    Lt *lt;

    Rect *rects;
    Color *colors;
    size_t rects_size;
};

Platforms *create_platforms_from_line_stream(LineStream *line_stream)
{
    trace_assert(line_stream);

    Lt *const lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    Platforms *platforms = PUSH_LT(lt, nth_alloc(sizeof(Platforms)), free);
    if (platforms == NULL) {
        RETURN_LT(lt, NULL);
    }

    platforms->rects_size = 0;
    if (sscanf(
            line_stream_next(line_stream),
            "%lu",
            &platforms->rects_size) == EOF) {
        log_fail("Could not read amount of platforms\n");
        RETURN_LT(lt, NULL);
    }

    platforms->rects = PUSH_LT(lt, nth_alloc(sizeof(Rect) * platforms->rects_size), free);
    if (platforms->rects == NULL) {
        RETURN_LT(lt, NULL);
    }

    platforms->colors = PUSH_LT(lt, nth_alloc(sizeof(Color) * platforms->rects_size), free);
    if (platforms->colors == NULL) {
        RETURN_LT(lt, NULL);
    }

    char color[7];
    for (size_t i = 0; i < platforms->rects_size; ++i) {
        if (sscanf(line_stream_next(line_stream),
                   "%f%f%f%f%6s\n",
                   &platforms->rects[i].x, &platforms->rects[i].y,
                   &platforms->rects[i].w, &platforms->rects[i].h,
                   color) < 0) {
            log_fail("Could not read %dth platform\n", i);
            RETURN_LT(lt, NULL);
        }
        platforms->colors[i] = hexstr(color);
    }

    platforms->lt = lt;

    return platforms;
}

void destroy_platforms(Platforms *platforms)
{
    trace_assert(platforms);
    RETURN_LT0(platforms->lt);
}

/* TODO(#450): platforms do not render their ids in debug mode */
int platforms_render(const Platforms *platforms,
                     Camera *camera)
{
    for (size_t i = 0; i < platforms->rects_size; ++i) {
        if (camera_fill_rect(
                camera,
                platforms->rects[i],
                platforms->colors[i]) < 0) {
            return -1;
        }
    }

    return 0;
}

void platforms_touches_rect_sides(const Platforms *platforms,
                                  Rect object,
                                  int sides[RECT_SIDE_N])
{
    trace_assert(platforms);

    for (size_t i = 0; i < platforms->rects_size; ++i) {
        rect_object_impact(object, platforms->rects[i], sides);
    }
}

Vec platforms_snap_rect(const Platforms *platforms,
                         Rect *object)
{
    trace_assert(platforms);

    Vec result = vec(1.0f, 1.0f);
    for (size_t i = 0; i < platforms->rects_size; ++i) {
        if (rects_overlap(platforms->rects[i], *object)) {
            result = vec_entry_mult(result, rect_snap(platforms->rects[i], object));
        }
    }

    return result;
}
