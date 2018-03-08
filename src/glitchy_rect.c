#include <assert.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "./glitchy_rect.h"
#include "./lt.h"
#include "./camera.h"
#include "./error.h"

#define GLITCHY_RECTS_COUNT 10
#define GLITCHY_RECTS_X_BUFFER 0.0f
#define GLITCHY_RECTS_Y_BUFFER 40.0f
#define GLITCHY_RECTS_INTERVAL 1500

struct glitchy_rect_t
{
    lt_t *lt;
    rect_t rect;
    color_t color;
    int glitchy;
    Uint32 timer;
    Uint32 interval;
    unsigned int seed;
};

glitchy_rect_t *create_glitchy_rect(rect_t rect, color_t color)
{
    lt_t *lt = create_lt();

    if (lt == NULL) {
        return NULL;
    }

    glitchy_rect_t * const glitchy_rect = PUSH_LT(
        lt,
        malloc(sizeof(glitchy_rect_t)),
        free);

    glitchy_rect->rect = rect;
    glitchy_rect->color = color;
    glitchy_rect->glitchy = 0;
    glitchy_rect->timer = 0;
    glitchy_rect->interval = (Uint32) (rand() % GLITCHY_RECTS_INTERVAL);
    glitchy_rect->seed = 0;
    glitchy_rect->lt = lt;

    return glitchy_rect;
}

glitchy_rect_t *create_glitchy_rect_from_stream(FILE *stream)
{
    char color_name[7];
    rect_t rect;

    if (fscanf(stream, "%f%f%f%f%6s\n",
               &rect.x, &rect.y,
               &rect.w, &rect.h,
               color_name) < 0) {
        throw_error(ERROR_TYPE_LIBC);
        return NULL;
    }

    color_t color = color_from_hexstr(color_name);

    return create_glitchy_rect(rect, color);
}

void destroy_glitchy_rect(glitchy_rect_t *glitchy_rect)
{
    assert(glitchy_rect);
    RETURN_LT0(glitchy_rect->lt);
}

int glitchy_rect_render(const glitchy_rect_t *glitchy_rect,
                        SDL_Renderer *renderer,
                        const camera_t *camera)
{
    assert(glitchy_rect);
    assert(renderer);
    assert(camera);

    if (glitchy_rect->glitchy) {
        srand(glitchy_rect->seed);

        const float x_range = glitchy_rect->rect.w + GLITCHY_RECTS_X_BUFFER;
        const float y_range = glitchy_rect->rect.h + GLITCHY_RECTS_Y_BUFFER;

        for (int i = 0; i < GLITCHY_RECTS_COUNT; i++) {
            const float x = fmodf((float) rand(), x_range) - GLITCHY_RECTS_X_BUFFER * 0.5f;
            const float y = fmodf((float) rand(), y_range) - GLITCHY_RECTS_Y_BUFFER * 0.5f;
            const float w = fmodf((float) rand(), (x_range - x)) - GLITCHY_RECTS_X_BUFFER * 0.5f;
            const float h = fmodf((float) rand(), (y_range - y)) - GLITCHY_RECTS_Y_BUFFER * 0.5f;

            if (camera_fill_rect(
                    camera,
                    renderer,
                    rect(
                        glitchy_rect->rect.x + x,
                        glitchy_rect->rect.y + y,
                        w,
                        h),
                    glitchy_rect->color) < 0) {
                return -1;
            }
        }

        srand((unsigned int) time(NULL));

    } else {
        return camera_fill_rect(
            camera,
            renderer,
            glitchy_rect->rect,
            glitchy_rect->color);
    }

    return 0;
}

int glitchy_rect_update(glitchy_rect_t *glitchy_rect,
                        Uint32 delta_time)
{
    assert(glitchy_rect);
    assert(delta_time > 0);

    glitchy_rect->timer += delta_time;

    if (!glitchy_rect->glitchy) {
        if (glitchy_rect->timer >= glitchy_rect->interval) {
            glitchy_rect->glitchy = 1;
            glitchy_rect->timer = 0;
            glitchy_rect->seed = (unsigned int) time(NULL) + (unsigned int) rand();
        }
    } else {
        if (glitchy_rect->timer >= 100) {
            glitchy_rect->glitchy = 0;
            glitchy_rect->timer = 0;
            glitchy_rect->interval = (Uint32) (rand() % GLITCHY_RECTS_INTERVAL);
        }
    }

    return 0;
}

int glitchy_rect_overlaps(const glitchy_rect_t *glitchy_rect,
                          rect_t rect)
{
    return rects_overlap(glitchy_rect->rect, rect);
}
