#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>

#include "camera.h"
#include "sdl/renderer.h"
#include "system/nth_alloc.h"
#include "system/log.h"

#define RATIO_X 16.0f
#define RATIO_Y 9.0f

struct Camera {
    bool debug_mode;
    bool blackwhite_mode;
    Point position;
    SDL_Renderer *renderer;
    Sprite_font *font;
};

static Vec effective_ratio(const SDL_Rect *view_port);
static Vec effective_scale(const SDL_Rect *view_port);
static Vec camera_point(const Camera *camera,
                        const SDL_Rect *view_port,
                        const Vec p);
static Rect camera_rect(const Camera *camera,
                        const SDL_Rect *view_port,
                        const Rect rect);
static Triangle camera_triangle(const Camera *camera,
                                  const SDL_Rect *view_port,
                                  const Triangle t);

Camera *create_camera(SDL_Renderer *renderer,
                      Sprite_font *font)
{
    Camera *camera = nth_alloc(sizeof(Camera));

    if (camera == NULL) {
        return NULL;
    }

    camera->position = vec(0.0f, 0.0f);
    camera->debug_mode = 0;
    camera->blackwhite_mode = 0;
    camera->renderer = renderer;
    camera->font = font;

    return camera;
}

void destroy_camera(Camera *camera)
{
    assert(camera);

    free(camera);
}

int camera_fill_rect(Camera *camera,
                     Rect rect,
                     Color color)
{
    assert(camera);

    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    const SDL_Rect sdl_rect = rect_for_sdl(
        camera_rect(camera, &view_port, rect));

    const SDL_Color sdl_color = color_for_sdl(camera->blackwhite_mode ? color_desaturate(color) : color);

    if (camera->debug_mode) {
        if (SDL_SetRenderDrawColor(camera->renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a / 2) < 0) {
            log_fail("SDL_SetRenderDrawColor: %s\n", SDL_GetError());
            return -1;
        }
    } else {
        if (SDL_SetRenderDrawColor(camera->renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a) < 0) {
            log_fail("SDL_SetRenderDrawColor: %s\n", SDL_GetError());
            return -1;
        }
    }

    if (SDL_RenderFillRect(camera->renderer, &sdl_rect) < 0) {
        log_fail("SDL_RenderFillRect: %s\n", SDL_GetError());
        return -1;
    }

    return 0;
}

int camera_draw_rect(Camera * camera,
                     Rect rect,
                     Color color)
{
    assert(camera);

    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    const SDL_Rect sdl_rect = rect_for_sdl(
        camera_rect(camera, &view_port, rect));

    const SDL_Color sdl_color = color_for_sdl(camera->blackwhite_mode ? color_desaturate(color) : color);

    if (SDL_SetRenderDrawColor(camera->renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a) < 0) {
        log_fail("SDL_SetRenderDrawColor: %s\n", SDL_GetError());
        return -1;
    }

    if (SDL_RenderDrawRect(camera->renderer, &sdl_rect) < 0) {
        log_fail("SDL_RenderDrawRect: %s\n", SDL_GetError());
        return -1;
    }

    return 0;
}

int camera_draw_triangle(Camera *camera,
                         Triangle t,
                         Color color)
{
    assert(camera);

    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    const SDL_Color sdl_color = color_for_sdl(camera->blackwhite_mode ? color_desaturate(color) : color);

    if (SDL_SetRenderDrawColor(camera->renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a) < 0) {
        log_fail("SDL_SetRenderDrawColor: %s\n", SDL_GetError());
        return -1;
    }

    if (draw_triangle(camera->renderer, camera_triangle(camera, &view_port, t)) < 0) {
        return -1;
    }

    return 0;
}

int camera_fill_triangle(Camera *camera,
                         Triangle t,
                         Color color)
{
    assert(camera);

    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    const SDL_Color sdl_color = color_for_sdl(camera->blackwhite_mode ? color_desaturate(color) : color);


    if (camera->debug_mode) {
        if (SDL_SetRenderDrawColor(camera->renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a / 2) < 0) {
            log_fail("SDL_SetRenderDrawColor: %s\n", SDL_GetError());
            return -1;
        }
    } else {
        if (SDL_SetRenderDrawColor(camera->renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a) < 0) {
            log_fail("SDL_SetRenderDrawColor: %s\n", SDL_GetError());
            return -1;
        }
    }

    if (fill_triangle(camera->renderer, camera_triangle(camera, &view_port, t)) < 0) {
        return -1;
    }

    return 0;
}

int camera_render_text(Camera *camera,
                       const char *text,
                       Vec size,
                       Color c,
                       Vec position)
{
    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    const Vec scale = effective_scale(&view_port);
    const Vec screen_position = camera_point(camera, &view_port, position);

    if (sprite_font_render_text(
            camera->font,
            camera->renderer,
            screen_position,
            vec(size.x * scale.x, size.y * scale.y),
            camera->blackwhite_mode ? color_desaturate(c) : c,
            text) < 0) {
        return -1;
    }

    return 0;
}

int camera_render_debug_text(Camera *camera,
                             const char *text,
                             Vec position)
{
    assert(camera);
    assert(text);

    if (!camera->debug_mode) {
        return 0;
    }

    if (camera_render_text(
            camera,
            text,
            vec(2.0f, 2.0f),
            color(0.0f, 0.0f, 0.0f, 1.0f),
            position) < 0) {
        return -1;
    }

    return 0;
}

int camera_clear_background(Camera *camera,
                            Color color)
{
    const SDL_Color sdl_color = color_for_sdl(camera->blackwhite_mode ? color_desaturate(color) : color);

    if (SDL_SetRenderDrawColor(camera->renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a) < 0) {
        log_fail("SDL_SetRenderDrawColor: %s\n", SDL_GetError());
        return -1;
    }

    if (SDL_RenderClear(camera->renderer) < 0) {
        log_fail("SDL_RenderClear: %s\n", SDL_GetError());
        return -1;
    }

    return 0;
}

void camera_center_at(Camera *camera, Point position)
{
    assert(camera);
    camera->position = position;
}

void camera_toggle_debug_mode(Camera *camera)
{
    assert(camera);
    camera->debug_mode = !camera->debug_mode;
}

void camera_disable_debug_mode(Camera *camera)
{
    assert(camera);
    camera->debug_mode = 0;
}

void camera_toggle_blackwhite_mode(Camera *camera)
{
    assert(camera);
    camera->blackwhite_mode = !camera->blackwhite_mode;
}

int camera_is_point_visible(const Camera *camera, Point p)
{
    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    return rect_contains_point(
        rect_from_sdl(&view_port),
        camera_point(camera, &view_port, p));
}

Rect camera_view_port(const Camera *camera)
{
    assert(camera);

    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    const Vec s = effective_scale(&view_port);
    const float w = (float) view_port.w * s.x;
    const float h = (float) view_port.h * s.y;

    return rect(camera->position.x - w * 0.5f,
                camera->position.y - h * 0.5f,
                w, h);
}

int camera_is_text_visible(const Camera *camera,
                           Vec size,
                           Vec position,
                           const char *text)
{
    assert(camera);
    assert(text);

    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    return rects_overlap(
        camera_rect(
            camera,
            &view_port,
            sprite_font_boundary_box(
                camera->font,
                position,
                size,
                text)),
        rect_from_sdl(&view_port));
}

/* ---------- Private Function ---------- */

static Vec effective_ratio(const SDL_Rect *view_port)
{
    if ((float) view_port->w / RATIO_X > (float) view_port->h / RATIO_Y) {
        return vec(RATIO_X, (float) view_port->h / ((float) view_port->w / RATIO_X));
    } else {
        return vec((float) view_port->w / ((float) view_port->h / RATIO_Y), RATIO_Y);
    }
}

static Vec effective_scale(const SDL_Rect *view_port)
{
    return vec_entry_div(
        vec((float) view_port->w, (float) view_port->h),
        vec_scala_mult(effective_ratio(view_port), 50.0f));
}

static Vec camera_point(const Camera *camera,
                          const SDL_Rect *view_port,
                          const Vec p)

{
    return vec_sum(
        vec_entry_mult(
            vec_sum(p, vec_neg(camera->position)),
            effective_scale(view_port)),
        vec((float) view_port->w * 0.5f,
            (float) view_port->h * 0.5f));
}

static Triangle camera_triangle(const Camera *camera,
                                  const SDL_Rect *view_port,
                                  const Triangle t)
{
    return triangle(
        camera_point(camera, view_port, t.p1),
        camera_point(camera, view_port, t.p2),
        camera_point(camera, view_port, t.p3));
}

static Rect camera_rect(const Camera *camera,
                          const SDL_Rect *view_port,
                          const Rect rect)
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

int camera_render_debug_rect(Camera *camera,
                             Rect rect,
                             Color c)
{
    assert(camera);

    if (!camera->debug_mode) {
        return 0;
    }

    if (camera_fill_rect(camera, rect, c) < 0) {
        return -1;
    }

    return 0;
}
