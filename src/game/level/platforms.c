#include <SDL.h>
#include "system/stacktrace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "platforms.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/log.h"
#include "game/level/level_editor/rect_layer.h"
#include "math/extrema.h"

struct Platforms {
    Lt *lt;

    Rect *rects;
    Color *colors;
    size_t rects_size;
};

Platforms *create_platforms_from_rect_layer(const RectLayer *layer)
{
    trace_assert(layer);

    Lt *lt = create_lt();

    Platforms *platforms = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(Platforms)),
        free);
    if (platforms == NULL) {
        RETURN_LT(lt, NULL);
    }
    platforms->lt = lt;

    platforms->rects_size = rect_layer_count(layer);

    platforms->rects = PUSH_LT(lt, nth_calloc(1, sizeof(Rect) * platforms->rects_size), free);
    if (platforms->rects == NULL) {
        RETURN_LT(lt, NULL);
    }
    memcpy(platforms->rects, rect_layer_rects(layer), sizeof(Rect) * platforms->rects_size);


    platforms->colors = PUSH_LT(lt, nth_calloc(1, sizeof(Color) * platforms->rects_size), free);
    if (platforms->colors == NULL) {
        RETURN_LT(lt, NULL);
    }
    memcpy(platforms->colors, rect_layer_colors(layer), sizeof(Color) * platforms->rects_size);

    return platforms;
}

void destroy_platforms(Platforms *platforms)
{
    trace_assert(platforms);
    RETURN_LT0(platforms->lt);
}

int platforms_render(const Platforms *platforms,
                     const Camera *camera)
{
    for (size_t i = 0; i < platforms->rects_size; ++i) {
        Rect platform_rect = platforms->rects[i];
        if (camera_fill_rect(
                camera,
                platform_rect,
                platforms->colors[i]) < 0) {
            return -1;
        }

        char debug_text[256];
        snprintf(debug_text, 256,
            "id:%zd\n"
            "x:%.2f\n"
            "y:%.2f\n"
            "w:%.2f\n"
            "h:%.2f\n",
            i, platform_rect.x, platform_rect.y, platform_rect.w, platform_rect.h);

        Vec2f text_pos = (Vec2f){.x = platform_rect.x, .y = platform_rect.y};
        Rect text_rect = sprite_font_boundary_box(text_pos, vec(2.0f, 2.0f), debug_text);

        Rect world_viewport = camera_view_port(camera);
        Rect viewport = camera_view_port_screen(camera);

        if (rects_overlap(
                camera_rect(
                    camera,
                    platform_rect),
                viewport) &&
            camera_is_point_visible(
                camera,
                text_pos) == false) {
            if (platform_rect.w > text_rect.w){
                text_pos.x = fmaxf(fminf(world_viewport.x, platform_rect.x + platform_rect.w - text_rect.w),
                                   platform_rect.x);
            }
            if (platform_rect.h > text_rect.h){
                text_pos.y = fmaxf(fminf(world_viewport.y, platform_rect.y + platform_rect.h - text_rect.h),
                                   platform_rect.y);
            }
        }

        if (camera_render_debug_text(
                camera,
                debug_text,
                text_pos) < 0) {
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

Vec2f platforms_snap_rect(const Platforms *platforms,
                         Rect *object)
{
    trace_assert(platforms);

    Vec2f result = vec(1.0f, 1.0f);
    for (size_t i = 0; i < platforms->rects_size; ++i) {
        if (rects_overlap(platforms->rects[i], *object)) {
            // TODO(#1161): can we reuse the Level Editor snapping mechanism in physics snapping
            result = vec_entry_mult(result, rect_snap(platforms->rects[i], object));
        }
    }

    return result;
}
