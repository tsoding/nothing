#ifndef CAMERA_H_
#define CAMERA_H_

#include <stdbool.h>

#include "color.h"
#include "game/sprite_font.h"
#include "math/vec.h"
#include "math/rect.h"
#include "math/triangle.h"
#include "config.h"

typedef struct {
    bool debug_mode;
    bool blackwhite_mode;
    Vec2f position;
    float scale;
    SDL_Renderer *renderer;
    Sprite_font font;
    Vec2f effective_scale;
} Camera;

Camera create_camera(SDL_Renderer *renderer,
                     Sprite_font font);

int camera_clear_background(const Camera *camera,
                            Color color);

int camera_fill_rect(const Camera *camera,
                     Rect rect,
                     Color color);

int camera_draw_rect(const Camera *camera,
                     Rect rect,
                     Color color);

int camera_draw_rect_screen(const Camera *camera,
                            Rect rect,
                            Color color);

int camera_draw_thicc_rect_screen(const Camera *camera,
                                  Rect rect,
                                  Color color,
                                  float thiccness);

int camera_draw_line(const Camera *camera,
                     Vec2f begin, Vec2f end,
                     Color color);

int camera_draw_triangle(Camera *camera,
                         Triangle t,
                         Color color);

int camera_fill_triangle(const Camera *camera,
                         Triangle t,
                         Color color);

int camera_render_text(const Camera *camera,
                       const char *text,
                       Vec2f size,
                       Color color,
                       Vec2f position);

void camera_render_text_screen(const Camera *camera,
                               const char *text,
                               Vec2f size,
                               Color color,
                               Vec2f position);

int camera_render_debug_text(const Camera *camera,
                             const char *text,
                             Vec2f position);

int camera_render_debug_rect(const Camera *camera,
                             Rect rect,
                             Color color);

void camera_center_at(Camera *camera, Vec2f position);
void camera_scale(Camera *came, float scale);

void camera_toggle_debug_mode(Camera *camera);
void camera_disable_debug_mode(Camera *camera);

int camera_is_point_visible(const Camera *camera, Vec2f p);
int camera_is_text_visible(const Camera *camera,
                           Vec2f size,
                           Vec2f position,
                           const char *text);

Rect camera_view_port(const Camera *camera);

Rect camera_view_port_screen(const Camera *camera);

Vec2f camera_map_screen(const Camera *camera,
                        Sint32 x, Sint32 y);

Vec2f camera_point(const Camera *camera, const Vec2f p);
Rect camera_rect(const Camera *camera, const Rect rect);

int camera_fill_rect_screen(const Camera *camera,
                            Rect rect,
                            Color color);


Vec2f effective_scale(const SDL_Rect *view_port);

#endif  // CAMERA_H_
