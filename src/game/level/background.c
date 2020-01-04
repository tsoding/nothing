#include <stdio.h>

#include "game/level/background.h"
#include "math/rand.h"
#include "math/rect.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/log.h"
#include "system/stacktrace.h"
#include "config.h"

static inline
Vec2i chunk_of_point(Vec2f p)
{
    return vec2i(
        (int) floorf(p.x / BACKGROUND_CHUNK_WIDTH),
        (int) floorf(p.y / BACKGROUND_CHUNK_HEIGHT));
}

int render_chunk(const Camera *camera,
                 Vec2i chunk,
                 Color color);

int background_render(const Background *background,
                      const Camera *camera0)
{
    trace_assert(background);
    trace_assert(camera0);

    Camera camera = *camera0;

    if (camera_clear_background(
            &camera,
            background->base_color) < 0) {
        return -1;
    }

    camera.scale = 1.0f - BACKGROUND_LAYERS_STEP * BACKGROUND_LAYERS_COUNT;

    for (int l = 0; l < BACKGROUND_LAYERS_COUNT; ++l) {
        const Rect view_port = camera_view_port(&camera);
        const Vec2f position = vec(view_port.x, view_port.y);

        Vec2i min = chunk_of_point(position);
        Vec2i max = chunk_of_point(vec_sum(position, vec(view_port.w, view_port.h)));

        for (int x = min.x - 1; x <= max.x; ++x) {
            for (int y = min.y - 1; y <= max.y; ++y) {
                if (render_chunk(
                        &camera,
                        vec2i(x, y),
                        color_darker(background->base_color, 0.05f * (float)(l + 1))) < 0) {
                    return -1;
                }
            }
        }

        camera.scale += BACKGROUND_LAYERS_STEP;
    }

    return 0;
}

/* Private Function */

int render_chunk(const Camera *camera,
                 Vec2i chunk,
                 Color color)
{
    trace_assert(camera);

    if (camera->debug_mode) {
        return 0;
    }

    srand((unsigned int)(roundf((float)chunk.x + (float)chunk.y + camera->scale * 10.0f)));

    for (size_t i = 0; i < BACKGROUND_TURDS_PER_CHUNK; ++i) {
        const float rect_x = rand_float_range(0.0f, BACKGROUND_CHUNK_WIDTH);
        const float rect_y = rand_float_range(0.0f, BACKGROUND_CHUNK_HEIGHT);

        const float rect_w = rand_float_range(0.0f, BACKGROUND_CHUNK_WIDTH * 0.5f);
        const float rect_h = rand_float_range(rect_w * 0.5f, rect_w * 1.5f);

        if (camera_fill_rect(
                camera,
                rect((float) chunk.x * BACKGROUND_CHUNK_WIDTH + rect_x,
                     (float) chunk.y * BACKGROUND_CHUNK_HEIGHT + rect_y,
                     rect_w,
                     rect_h),
                color) < 0) {
            return -1;
        }
    }

    return 0;
}

Color background_base_color(const Background *background)
{
    return background->base_color;
}
