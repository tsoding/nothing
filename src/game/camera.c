#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "camera.h"
#include "sdl/renderer.h"
#include "system/nth_alloc.h"
#include "system/log.h"
#include "system/stacktrace.h"

static Triangle camera_triangle(const Camera *camera,
                                const Triangle t);

static SDL_Color camera_sdl_color(const Camera *camera, Color color)
{
    return color_for_sdl(camera->blackwhite_mode ? color_desaturate(color) : color);
}

Camera create_camera(SDL_Renderer *renderer,
                     Sprite_font font)
{
    trace_assert(renderer);

    Camera camera = {
        .scale = 1.0f,
        .renderer = renderer,
        .font = font
    };

    return camera;
}

int camera_fill_rect(const Camera *camera,
                     Rect rect,
                     Color color)
{
    trace_assert(camera);

    const SDL_Rect sdl_rect = rect_for_sdl(
        camera_rect(camera, rect));

    const SDL_Color sdl_color = camera_sdl_color(camera, color);

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

int camera_draw_rect(const Camera *camera,
                     Rect rect,
                     Color color)
{
    trace_assert(camera);

    const SDL_Rect sdl_rect = rect_for_sdl(
        camera_rect(camera, rect));

    const SDL_Color sdl_color = camera_sdl_color(camera, color);

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

int camera_draw_rect_screen(const Camera *camera,
                            Rect rect,
                            Color color)
{
    trace_assert(camera);

    const SDL_Rect sdl_rect = rect_for_sdl(rect);
    const SDL_Color sdl_color = camera_sdl_color(camera, color);

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
    trace_assert(camera);

    const SDL_Color sdl_color = camera_sdl_color(camera, color);

    if (SDL_SetRenderDrawColor(camera->renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a) < 0) {
        log_fail("SDL_SetRenderDrawColor: %s\n", SDL_GetError());
        return -1;
    }

    if (draw_triangle(camera->renderer, camera_triangle(camera, t)) < 0) {
        return -1;
    }

    return 0;
}

int camera_fill_triangle(const Camera *camera,
                         Triangle t,
                         Color color)
{
    trace_assert(camera);

    const SDL_Color sdl_color = camera_sdl_color(camera, color);

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

    if (fill_triangle(camera->renderer, camera_triangle(camera, t)) < 0) {
        return -1;
    }

    return 0;
}

int camera_render_text(const Camera *camera,
                       const char *text,
                       Vec2f size,
                       Color c,
                       Vec2f position)
{
    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    const Vec2f scale = camera->effective_scale;
    const Vec2f screen_position = camera_point(camera, position);

    sprite_font_render_text(
        &camera->font,
        camera->renderer,
        screen_position,
        vec(size.x * scale.x * camera->scale, size.y * scale.y * camera->scale),
        camera->blackwhite_mode ? color_desaturate(c) : c,
        text);

    return 0;
}

int camera_render_debug_text(const Camera *camera,
                             const char *text,
                             Vec2f position)
{
    trace_assert(camera);
    trace_assert(text);

    if (!camera->debug_mode) {
        return 0;
    }

    if (camera_render_text(
            camera,
            text,
            vec(2.0f, 2.0f),
            rgba(0.0f, 0.0f, 0.0f, 1.0f),
            position) < 0) {
        return -1;
    }

    return 0;
}

int camera_clear_background(const Camera *camera,
                            Color color)
{
    const SDL_Color sdl_color = camera_sdl_color(camera, color);

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

void camera_center_at(Camera *camera, Vec2f position)
{
    trace_assert(camera);
    camera->position = position;
}

void camera_scale(Camera *camera, float scale)
{
    trace_assert(camera);
    camera->scale = fmaxf(0.1f, scale);
}

void camera_toggle_debug_mode(Camera *camera)
{
    trace_assert(camera);
    camera->debug_mode = !camera->debug_mode;
}

void camera_disable_debug_mode(Camera *camera)
{
    trace_assert(camera);
    camera->debug_mode = 0;
}

int camera_is_point_visible(const Camera *camera, Vec2f p)
{
    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    return rect_contains_point(
        rect_from_sdl(&view_port),
        camera_point(camera, p));
}

Rect camera_view_port(const Camera *camera)
{
    trace_assert(camera);

    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    Vec2f p1 = camera_map_screen(
        camera,
        view_port.x,
        view_port.y);
    Vec2f p2 = camera_map_screen(
        camera,
        view_port.x + view_port.w,
        view_port.y + view_port.h);

    return rect_from_points(p1, p2);
}

Rect camera_view_port_screen(const Camera *camera)
{
    trace_assert(camera);

    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    return rect_from_sdl(&view_port);
}

int camera_is_text_visible(const Camera *camera,
                           Vec2f size,
                           Vec2f position,
                           const char *text)
{
    trace_assert(camera);
    trace_assert(text);

    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    return rects_overlap(
        camera_rect(
            camera,
            sprite_font_boundary_box(position, size, text)),
        rect_from_sdl(&view_port));
}

/* ---------- Private Function ---------- */

Vec2f camera_point(const Camera *camera, const Vec2f p)
{
    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    return vec_sum(
        vec_scala_mult(
            vec_entry_mult(
                vec_sum(p, vec_neg(camera->position)),
                camera->effective_scale),
            camera->scale),
        vec((float) view_port.w * 0.5f,
            (float) view_port.h * 0.5f));
}

static Triangle camera_triangle(const Camera *camera,
                                const Triangle t)
{
    return triangle(
        camera_point(camera, t.p1),
        camera_point(camera, t.p2),
        camera_point(camera, t.p3));
}

Rect camera_rect(const Camera *camera, const Rect rect)
{
    return rect_from_points(
        camera_point(
            camera,
            vec(rect.x, rect.y)),
        camera_point(
            camera,
            vec(rect.x + rect.w, rect.y + rect.h)));
}

int camera_render_debug_rect(const Camera *camera,
                             Rect rect,
                             Color c)
{
    trace_assert(camera);

    if (!camera->debug_mode) {
        return 0;
    }

    if (camera_fill_rect(camera, rect, c) < 0) {
        return -1;
    }

    return 0;
}

Vec2f camera_map_screen(const Camera *camera,
                      Sint32 x, Sint32 y)
{
    trace_assert(camera);

    SDL_Rect view_port;
    SDL_RenderGetViewport(camera->renderer, &view_port);

    Vec2f es = camera->effective_scale;
    es.x = 1.0f / es.x;
    es.y = 1.0f / es.y;

    const Vec2f p = vec((float) x, (float) y);

    return vec_sum(
        vec_entry_mult(
            vec_scala_mult(
                vec_sum(
                    p,
                    vec((float) view_port.w * -0.5f,
                        (float) view_port.h * -0.5f)),
                1.0f / camera->scale),
            es),
        camera->position);
}

int camera_fill_rect_screen(const Camera *camera,
                            Rect rect,
                            Color color)
{
    trace_assert(camera);

    const SDL_Rect sdl_rect = rect_for_sdl(rect);
    const SDL_Color sdl_color = camera_sdl_color(camera, color);

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

void camera_render_text_screen(const Camera *camera,
                               const char *text,
                               Vec2f size,
                               Color color,
                               Vec2f position)
{
    trace_assert(camera);
    trace_assert(text);

    sprite_font_render_text(
        &camera->font,
        camera->renderer,
        position,
        size,
        color,
        text);
}

int camera_draw_thicc_rect_screen(const Camera *camera,
                                  Rect rect,
                                  Color color,
                                  float thiccness)
{
    trace_assert(camera);

    // Top
    if (camera_fill_rect_screen(
            camera,
            horizontal_thicc_line(
               rect.x,
               rect.x + rect.w,
               rect.y,
               thiccness),
            color) < 0) {
        return -1;
    }

    // Bottom
    if (camera_fill_rect_screen(
            camera,
            horizontal_thicc_line(
                rect.x,
                rect.x + rect.w,
                rect.y + rect.h,
                thiccness),
            color) < 0) {
        return -1;
    }

    // Left
    if (camera_fill_rect_screen(
            camera,
            vertical_thicc_line(
                rect.y,
                rect.y + rect.h,
                rect.x,
                thiccness),
            color) < 0) {
        return -1;
    }

    // Right
    if (camera_fill_rect_screen(
            camera,
            vertical_thicc_line(
                rect.y,
                rect.y + rect.h,
                rect.x + rect.w,
                thiccness),
            color) < 0) {
        return -1;
    }

    return 0;
}

int camera_draw_line(const Camera *camera,
                     Vec2f begin, Vec2f end,
                     Color color)
{
    trace_assert(camera);

    const Vec2f camera_begin = camera_point(camera, begin);
    const Vec2f camera_end = camera_point(camera, end);

    const SDL_Color sdl_color = camera_sdl_color(camera, color);

    if (SDL_SetRenderDrawColor(camera->renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a) < 0) {
        log_fail("SDL_SetRenderDrawColor: %s\n", SDL_GetError());
        return -1;
    }

    if (SDL_RenderDrawLine(
            camera->renderer,
            (int)roundf(camera_begin.x),
            (int)roundf(camera_begin.y),
            (int)roundf(camera_end.x),
            (int)roundf(camera_end.y)) < 0) {
        log_fail("SDL_RenderDrawRect: %s\n", SDL_GetError());
        return -1;
    }

    return 0;
}

static inline
Vec2f effective_ratio(const SDL_Rect *view_port)
{
    if ((float) view_port->w / CAMERA_RATIO_X > (float) view_port->h / CAMERA_RATIO_Y) {
        return vec(CAMERA_RATIO_X, (float) view_port->h / ((float) view_port->w / CAMERA_RATIO_X));
    } else {
        return vec((float) view_port->w / ((float) view_port->h / CAMERA_RATIO_Y), CAMERA_RATIO_Y);
    }
}

Vec2f effective_scale(const SDL_Rect *view_port)
{
    return vec_entry_div(
        vec((float) view_port->w, (float) view_port->h),
        vec_scala_mult(effective_ratio(view_port), 50.0f));
}
