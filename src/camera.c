#include <SDL2/SDL.h>

#include <math.h>
#include <assert.h>

#include "./camera.h"
#include "./error.h"
#include "./renderer.h"

#define RATIO_X 16.0f
#define RATIO_Y 9.0f

struct camera_t {
    int debug_mode;
    point_t position;
};

static vec_t effective_ratio(const SDL_Rect *view_port);
static vec_t effective_scale(const SDL_Rect *view_port);
static vec_t camera_point(const camera_t *camera,
                          const SDL_Rect *view_port,
                          const vec_t p);
static rect_t camera_rect(const camera_t *camera,
                          const SDL_Rect *view_port,
                          const rect_t rect);

camera_t *create_camera(point_t position)
{
    camera_t *camera = malloc(sizeof(camera_t));

    if (camera == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        return NULL;
    }

    camera->position = position;
    camera->debug_mode = 0;

    return camera;
}

void destroy_camera(camera_t *camera)
{
    assert(camera);

    free(camera);
}


int camera_fill_rect(const camera_t *camera,
                     SDL_Renderer *render,
                     const rect_t *rect)
{
    assert(camera);
    assert(render);
    assert(rect);

    SDL_Rect view_port;
    SDL_RenderGetViewport(render, &view_port);

    const SDL_Rect sdl_rect = rect_for_sdl(
        camera_rect(camera, &view_port, *rect));

    if (camera->debug_mode) {
        if (SDL_RenderDrawRect(render, &sdl_rect) < 0) {
            throw_error(ERROR_TYPE_SDL2);
            return -1;
        }
    } else {
        if (SDL_RenderFillRect(render, &sdl_rect) < 0) {
            throw_error(ERROR_TYPE_SDL2);
            return -1;
        }
    }

    return 0;
}

int camera_draw_rect(const camera_t * camera,
                     SDL_Renderer *render,
                     const rect_t *rect)
{
    assert(camera);
    assert(render);
    assert(rect);

    SDL_Rect view_port;
    SDL_RenderGetViewport(render, &view_port);

    const SDL_Rect sdl_rect = rect_for_sdl(
        camera_rect(camera, &view_port, *rect));

    if (SDL_RenderDrawRect(render, &sdl_rect) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    return 0;
}

int camera_draw_triangle(const camera_t *camera,
                         SDL_Renderer *render,
                         point_t p1,
                         point_t p2,
                         point_t p3)
{
    assert(camera);
    assert(render);

    SDL_Rect view_port;
    SDL_RenderGetViewport(render, &view_port);

    if (draw_triangle(render,
                      camera_point(camera, &view_port, p1),
                      camera_point(camera, &view_port, p2),
                      camera_point(camera, &view_port, p3)) < 0) {
        return -1;
    }

    return 0;
}

int camera_fill_triangle(const camera_t *camera,
                         SDL_Renderer *render,
                         point_t p1,
                         point_t p2,
                         point_t p3)
{
    assert(camera);
    assert(render);

    SDL_Rect view_port;
    SDL_RenderGetViewport(render, &view_port);

    if (fill_triangle(render,
                      camera_point(camera, &view_port, p1),
                      camera_point(camera, &view_port, p2),
                      camera_point(camera, &view_port, p3)) < 0) {
        return -1;
    }

    return 0;
}

void camera_center_at(camera_t *camera, point_t position)
{
    assert(camera);
    camera->position = position;
}

void camera_toggle_debug_mode(camera_t *camera)
{
    assert(camera);
    camera->debug_mode = !camera->debug_mode;
}

/* ---------- Private Function ---------- */

static vec_t effective_ratio(const SDL_Rect *view_port)
{
    if ((float) view_port->w / RATIO_X > (float) view_port->h / RATIO_Y) {
        return vec(RATIO_X, (float) view_port->h / ((float) view_port->w / RATIO_X));
    } else {
        return vec((float) view_port->w / ((float) view_port->h / RATIO_Y), RATIO_Y);
    }
}

static vec_t effective_scale(const SDL_Rect *view_port)
{
    return vec_entry_div(
        vec((float) view_port->w, (float) view_port->h),
        vec_scala_mult(effective_ratio(view_port), 50.0f));
}

static vec_t camera_point(const camera_t *camera,
                          const SDL_Rect *view_port,
                          const vec_t p)

{
    return vec_sum(
        vec_entry_mult(
            vec_sum(p, vec_neg(camera->position)),
            effective_scale(view_port)),
        vec((float) view_port->w * 0.5f,
            (float) view_port->h * 0.5f));
}

static rect_t camera_rect(const camera_t *camera,
                          const SDL_Rect *view_port,
                          const rect_t rect)
{
    const point_t position =
        camera_point(
            camera,
            view_port,
            vec(rect.x, rect.y));

    const vec_t scale = effective_scale(view_port);

    const rect_t cam_rect = {
        .x = position.x,
        .y = position.y,
        .w = roundf(rect.w * scale.x),
        .h = roundf(rect.h * scale.y)
    };

    return cam_rect;
}
