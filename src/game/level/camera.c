#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>

#include "camera.h"
#include "sdl/renderer.h"
#include "system/error.h"

#define RATIO_X 16.0f
#define RATIO_Y 9.0f

struct camera_t {
    int debug_mode;
    int blackwhite_mode;
    point_t position;
    SDL_Renderer *renderer;
};

static vec_t effective_ratio(const SDL_Rect *view_port);
static vec_t effective_scale(const SDL_Rect *view_port);
static vec_t camera_point(const camera_t *camera,
                          const SDL_Rect *view_port,
                          const vec_t p);
static rect_t camera_rect(const camera_t *camera,
                          const SDL_Rect *view_port,
                          const rect_t rect);
static triangle_t camera_triangle(const camera_t *camera,
                                  const SDL_Rect *view_port,
                                  const triangle_t t);

camera_t *create_camera(void)
{
    camera_t *camera = malloc(sizeof(camera_t));

    if (camera == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        return NULL;
    }

    camera->position = vec(0.0f, 0.0f);
    camera->debug_mode = 0;
    camera->blackwhite_mode = 0;
    camera->renderer = NULL;

    return camera;
}

camera_t *create_camera_from_renderer(SDL_Renderer *renderer)
{
    camera_t *camera = create_camera();
    if (camera == NULL) {
        return NULL;
    }

    camera->renderer = renderer;

    return camera;
}

void destroy_camera(camera_t *camera)
{
    assert(camera);

    free(camera);
}


int camera_fill_rect(const camera_t *camera,
                     rect_t rect,
                     color_t color)
{
    assert(camera);

    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    const SDL_Rect sdl_rect = rect_for_sdl(
        camera_rect(camera, &view_port, rect));

    const SDL_Color sdl_color = color_for_sdl(camera->blackwhite_mode ? color_desaturate(color) : color);

    if (SDL_SetRenderDrawColor(camera->renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    if (camera->debug_mode) {
        if (SDL_RenderDrawRect(camera->renderer, &sdl_rect) < 0) {
            throw_error(ERROR_TYPE_SDL2);
            return -1;
        }
    } else {
        if (SDL_RenderFillRect(camera->renderer, &sdl_rect) < 0) {
            throw_error(ERROR_TYPE_SDL2);
            return -1;
        }
    }

    return 0;
}

int camera_draw_rect(const camera_t * camera,
                     rect_t rect,
                     color_t color)
{
    assert(camera);

    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    const SDL_Rect sdl_rect = rect_for_sdl(
        camera_rect(camera, &view_port, rect));

    const SDL_Color sdl_color = color_for_sdl(camera->blackwhite_mode ? color_desaturate(color) : color);

    if (SDL_SetRenderDrawColor(camera->renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    if (SDL_RenderDrawRect(camera->renderer, &sdl_rect) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    return 0;
}

int camera_draw_triangle(const camera_t *camera,
                         triangle_t t,
                         color_t color)
{
    assert(camera);

    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    const SDL_Color sdl_color = color_for_sdl(camera->blackwhite_mode ? color_desaturate(color) : color);

    if (SDL_SetRenderDrawColor(camera->renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    if (draw_triangle(camera->renderer, camera_triangle(camera, &view_port, t)) < 0) {
        return -1;
    }

    return 0;
}

int camera_fill_triangle(const camera_t *camera,
                         triangle_t t,
                         color_t color)
{
    assert(camera);

    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    const SDL_Color sdl_color = color_for_sdl(camera->blackwhite_mode ? color_desaturate(color) : color);

    if (SDL_SetRenderDrawColor(camera->renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    if (camera->debug_mode) {
        if (draw_triangle(camera->renderer, camera_triangle(camera, &view_port, t)) < 0) {
            return -1;
        }

    } else {
        if (fill_triangle(camera->renderer, camera_triangle(camera, &view_port, t)) < 0) {
            return -1;
        }
    }

    return 0;
}

int camera_clear_background(const camera_t *camera,
                            color_t color)
{
    const SDL_Color sdl_color = color_for_sdl(camera->blackwhite_mode ? color_desaturate(color) : color);

    if (SDL_SetRenderDrawColor(camera->renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    if (SDL_RenderClear(camera->renderer) < 0) {
        throw_error(ERROR_TYPE_SDL2);
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

void camera_toggle_blackwhite_mode(camera_t *camera)
{
    assert(camera);
    camera->blackwhite_mode = !camera->blackwhite_mode;
}

int camera_is_point_visible(const camera_t *camera, SDL_Renderer *renderer, point_t p)
{
    SDL_Rect view_port;
    SDL_RenderGetViewport(renderer, &view_port);

    return rect_contains_point(
        rect_from_sdl(&view_port),
        camera_point(camera, &view_port, p));
}

rect_t camera_view_port(const camera_t *camera,
                        SDL_Renderer *renderer)
{
    assert(camera);
    assert(renderer);

    SDL_Rect view_port;
    SDL_RenderGetViewport(renderer, &view_port);

    const vec_t s = effective_scale(&view_port);
    const float w = (float) view_port.w * s.x;
    const float h = (float) view_port.h * s.y;

    return rect(camera->position.x - w * 0.5f,
                camera->position.y - h * 0.5f,
                w, h);
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

static triangle_t camera_triangle(const camera_t *camera,
                                  const SDL_Rect *view_port,
                                  const triangle_t t)
{
    return triangle(
        camera_point(camera, view_port, t.p1),
        camera_point(camera, view_port, t.p2),
        camera_point(camera, view_port, t.p3));
}

static rect_t camera_rect(const camera_t *camera,
                          const SDL_Rect *view_port,
                          const rect_t rect)
{
    return rect_from_vecs(
        camera_point(
            camera,
            view_port,
            vec(rect.x, rect.y)),
        vec_entry_mult(
            effective_scale(view_port),
            vec(rect.w, rect.h)));
}
