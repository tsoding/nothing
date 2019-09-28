#include <stdio.h>

#include "game/level/background.h"
#include "math/rand.h"
#include "math/rect.h"
#include "system/line_stream.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/log.h"
#include "system/stacktrace.h"
#include "config.h"

typedef struct {
    int x, y;
} Vec2i;

static inline
Vec2i vec2i(int x, int y)
{
    Vec2i resoolt = {
        .x = x,
        .y = y
    };
    return resoolt;
}

static inline
Vec2i chunk_of_point(Vec2f p)
{
    return vec2i(
        (int) floorf(p.x / BACKGROUND_CHUNK_WIDTH),
        (int) floorf(p.y / BACKGROUND_CHUNK_HEIGHT));
}

int render_chunk(const Background *background,
                 const Camera *camera,
                 Vec2i chunk,
                 Color color);

struct Background
{
    Lt *lt;
    Color base_color;
    int debug_mode;
};

Background *create_background(Color base_color)
{
    Lt *lt = create_lt();

    Background *background = PUSH_LT(lt, nth_calloc(1, sizeof(Background)), free);
    if (background == NULL) {
        RETURN_LT(lt, NULL);
    }

    background->base_color = base_color;
    background->debug_mode = 0;
    background->lt = lt;

    return background;
}

Background *create_background_from_line_stream(LineStream *line_stream)
{
    char color[7];
    if (sscanf(line_stream_next(line_stream), "%6s", color) == EOF) {
        log_fail("Could not read background's color\n");
        return NULL;
    }

    return create_background(hexstr(color));
}

void destroy_background(Background *background)
{
    trace_assert(background);
    RETURN_LT0(background->lt);
}

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
                        background,
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

int render_chunk(const Background *background,
                 const Camera *camera,
                 Vec2i chunk,
                 Color color)
{
    (void) color;

    if (background->debug_mode) {
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

void background_toggle_debug_mode(Background *background)
{
    background->debug_mode = !background->debug_mode;
}

Color background_base_color(const Background *background)
{
    return background->base_color;
}
