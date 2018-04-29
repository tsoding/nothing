#ifndef CAMERA_H_
#define CAMERA_H_

#include "./color.h"
#include "./math/point.h"
#include "./math/rect.h"
#include "./math/triangle.h"

typedef struct camera_t camera_t;

camera_t *create_camera(point_t position);
void destroy_camera(camera_t *camera);

int camera_clear_background(const camera_t *camera,
                            SDL_Renderer *render,
                            color_t color);

int camera_fill_rect(const camera_t *camera,
                     SDL_Renderer *render,
                     rect_t rect,
                     color_t color);

int camera_draw_rect(const camera_t * camera,
                     SDL_Renderer *render,
                     rect_t rect,
                     color_t color);

int camera_draw_triangle(const camera_t *camera,
                         SDL_Renderer *render,
                         triangle_t t,
                         color_t color);

int camera_fill_triangle(const camera_t *camera,
                         SDL_Renderer *render,
                         triangle_t t,
                         color_t color);

void camera_center_at(camera_t *camera, point_t position);

void camera_toggle_debug_mode(camera_t *camera);

void camera_toggle_blackwhite_mode(camera_t *camera);

int camera_is_point_visible(const camera_t *camera, SDL_Renderer *renderer, point_t p);

#endif  // CAMERA_H_
