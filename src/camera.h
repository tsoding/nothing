#ifndef CAMERA_H_
#define CAMERA_H_

#include "./rect.h"
#include "./point.h"
#include "./triangle.h"

typedef struct camera_t camera_t;

camera_t *create_camera(point_t position);
void destroy_camera(camera_t *camera);

int camera_fill_rect(const camera_t *camera,
                     SDL_Renderer *render,
                     const rect_t *rect);
int camera_draw_rect(const camera_t * camera,
                     SDL_Renderer *render,
                     const rect_t *rect);

int camera_draw_triangle(const camera_t *camera,
                         SDL_Renderer *render,
                         triangle_t t);

int camera_fill_triangle(const camera_t *camera,
                         SDL_Renderer *render,
                         triangle_t t);

void camera_center_at(camera_t *camera, point_t position);

void camera_toggle_debug_mode(camera_t *camera);

#endif  // CAMERA_H_
