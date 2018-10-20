#ifndef CAMERA_H_
#define CAMERA_H_

#include "color.h"
#include "game/sprite_font.h"
#include "math/point.h"
#include "math/rect.h"
#include "math/triangle.h"

typedef struct Camera Camera;

Camera *create_camera(SDL_Renderer *renderer,
                        Sprite_font *font);
void destroy_camera(Camera *camera);

int camera_clear_background(Camera *camera,
                            Color color);

int camera_fill_rect(Camera *camera,
                     Rect rect,
                     Color color);

int camera_draw_rect(Camera * camera,
                     Rect rect,
                     Color color);

int camera_draw_triangle(Camera *camera,
                         Triangle t,
                         Color color);

int camera_fill_triangle(Camera *camera,
                         Triangle t,
                         Color color);

int camera_render_text(Camera *camera,
                       const char *text,
                       Vec size,
                       Color color,
                       Vec position);

int camera_render_debug_text(Camera *camera,
                             const char *text,
                             Vec position);

void camera_center_at(Camera *camera, Point position);

void camera_toggle_debug_mode(Camera *camera);
void camera_disable_debug_mode(Camera *camera);

void camera_toggle_blackwhite_mode(Camera *camera);

int camera_is_point_visible(const Camera *camera, Point p);
int camera_is_text_visible(const Camera *camera,
                           Vec size,
                           Vec position,
                           const char *text);

Rect camera_view_port(const Camera *camera);

#endif  // CAMERA_H_
