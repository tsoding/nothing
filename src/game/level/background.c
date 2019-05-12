#include "system/stacktrace.h"

#include "game/level/background.h"
#include "math/rand.h"
#include "math/rect.h"
#include "system/line_stream.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/log.h"

#define BACKGROUND_CHUNK_COUNT 5
#define BACKGROUND_CHUNK_WIDTH 250.0f
#define BACKGROUND_CHUNK_HEIGHT 250.0f

static void chunk_of_point(Point p, int *x, int *y);
int render_chunk(const Background *background,
                 Camera *camera,
                 int x, int y,
                 Color color,
                 Vec position,
                 float parallax);

struct Background
{
    Lt lt;
    Color base_color;
    Vec position;
    int debug_mode;
};

Background *create_background(Color base_color)
{
    Lt lt = {0};

    Background *background = PUSH_LT(lt, nth_calloc(1, sizeof(Background)), free);
    if (background == NULL) {
        RETURN_LT(lt, NULL);
    }

    background->base_color = base_color;
    background->position = vec(0.0f, 0.0f);
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

/* TODO(#182): background chunks are randomly disappearing when the size of the window is less than size of the chunk  */
int background_render(const Background *background,
                      Camera *camera)
{
    trace_assert(background);
    trace_assert(camera);

    if (camera_clear_background(
            camera,
            background->base_color) < 0) {
        return -1;
    }

    const Rect view_port = camera_view_port(camera);
    const Vec position = vec(view_port.x, view_port.y);

    for (int l = 0; l < 3; ++l) {
        const float parallax = 1.0f - 0.2f * (float)l;

        int min_x = 0, min_y = 0;
        chunk_of_point(vec(view_port.x - position.x * parallax,
                           view_port.y - position.y * parallax),
                       &min_x, &min_y);

        int max_x = 0, max_y = 0;
        chunk_of_point(vec(view_port.x - position.x * parallax + view_port.w,
                           view_port.y - position.y * parallax + view_port.h),
                       &max_x, &max_y);

        for (int x = min_x; x <= max_x; ++x) {
            for (int y = min_y; y <= max_y; ++y) {
                if (render_chunk(
                        background,
                        camera,
                        x, y,
                        color_darker(background->base_color, 0.05f * (float)(l + 1)),
                        position,
                        parallax) < 0) {
                    return -1;
                }
            }
        }
    }

    return 0;
}

/* Private Function */

static void chunk_of_point(Point p, int *x, int *y)
{
    trace_assert(x);
    trace_assert(y);
    *x = (int) (p.x / BACKGROUND_CHUNK_WIDTH);
    *y = (int) (p.y / BACKGROUND_CHUNK_HEIGHT);
}

int render_chunk(const Background *background,
                 Camera *camera,
                 int chunk_x, int chunk_y,
                 Color color,
                 Vec position,
                 float parallax)
{
    (void) background;

    if (background->debug_mode) {
        return 0;
    }

    srand((unsigned int)(roundf((float)chunk_x + (float)chunk_y + parallax)));

    for (size_t i = 0; i < BACKGROUND_CHUNK_COUNT; ++i) {
        const float rect_x = rand_float_range((float) chunk_x * BACKGROUND_CHUNK_WIDTH,
                                              (float) (chunk_x + 1) * BACKGROUND_CHUNK_WIDTH);
        const float rect_y = rand_float_range((float) chunk_y * BACKGROUND_CHUNK_HEIGHT,
                                              (float) (chunk_y + 1) * BACKGROUND_CHUNK_HEIGHT);
        const float rect_w = rand_float_range(0.0f, BACKGROUND_CHUNK_WIDTH * 0.5f);
        const float rect_h = rand_float_range(rect_w * 0.5f, rect_w * 1.5f);

        if (camera_fill_rect(
                camera,
                rect(rect_x + position.x * parallax,
                     rect_y + position.y * parallax,
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
