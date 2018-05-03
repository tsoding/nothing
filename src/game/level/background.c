#include <assert.h>

#include "game/level/background.h"
#include "math/rand.h"
#include "math/rect.h"
#include "system/lt.h"

#define BACKGROUND_CHUNK_COUNT 10
#define BACKGROUND_CHUNK_WIDTH 500.0f
#define BACKGROUND_CHUNK_HEIGHT 500.0f

static void chunk_of_point(point_t p, int *x, int *y);
int render_chunk(const background_t *background,
                 const camera_t *camera,
                 int x, int y,
                 color_t color,
                 float parallax);

struct background_t
{
    lt_t *lt;
    color_t base_color;
    vec_t position;
};

background_t *create_background(color_t base_color)
{
    lt_t *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    background_t *background = PUSH_LT(lt, malloc(sizeof(background_t)), free);
    if (background == NULL) {
        RETURN_LT(lt, NULL);
    }

    background->base_color = base_color;
    background->position = vec(0.0f, 0.0f);
    background->lt = lt;

    return background;
}

void destroy_background(background_t *background)
{
    assert(background);
    RETURN_LT0(background->lt);
}

int background_render(const background_t *background,
                      const camera_t *camera)
{
    assert(background);
    assert(camera);

    rect_t view_port = camera_view_port(camera);

    int min_x = 0, min_y = 0;
    chunk_of_point(vec(view_port.x, view_port.y), &min_x, &min_y);

    int max_x = 0, max_y = 0;
    chunk_of_point(vec(view_port.x + view_port.w,
                       view_port.y + view_port.h), &max_x, &max_y);

    for (int l = 0; l < 3; ++l) {
        for (int x = min_x; x <= max_x; ++x) {
            for (int y = min_y; y <= max_y; ++y) {
                if (render_chunk(background, camera, x, y, color_darker(background->base_color, 0.2f * (float)l), 1.0f - 0.2f * (float)l) < 0) {
                    return -1;
                }
            }
        }
    }

    return 0;
}

void background_move_to(background_t *background,
                        vec_t position)
{
    background->position = position;
}

/* Private Function */

static void chunk_of_point(point_t p, int *x, int *y)
{
    assert(x);
    assert(y);
    *x = (int) roundf(p.x / BACKGROUND_CHUNK_WIDTH);
    *y = (int) roundf(p.y / BACKGROUND_CHUNK_HEIGHT);
}

int render_chunk(const background_t *background,
                 const camera_t *camera,
                 int x, int y,
                 color_t color,
                 float parallax)
{
    srand((unsigned int)(roundf((float)x + (float)y + parallax)));

    for (size_t i = 0; i < BACKGROUND_CHUNK_COUNT; ++i) {
        const float rect_x = rand_float_range((float) x * BACKGROUND_CHUNK_WIDTH,
                                              (float) (x + 1) * BACKGROUND_CHUNK_WIDTH);
        const float rect_y = rand_float_range((float) y * BACKGROUND_CHUNK_HEIGHT,
                                              (float) (y + 1) * BACKGROUND_CHUNK_HEIGHT);
        const float rect_w = rand_float_range(0.0f, BACKGROUND_CHUNK_WIDTH * 0.5f);
        const float rect_h = rand_float_range(0.0f, BACKGROUND_CHUNK_HEIGHT * 0.5f);

        if (camera_fill_rect(
                camera,
                rect(rect_x + background->position.x * parallax,
                     rect_y + background->position.y * parallax,
                     rect_w,
                     rect_h),
                color) < 0) {
            return -1;
        }
    }

    return 0;
}
