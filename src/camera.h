#ifndef CAMERA_H_
#define CAMERA_H_

#include "./rect.h"
#include "./point.h"

typedef struct camera_t camera_t;

camera_t *create_camera(point_t position);
void destroy_camera(camera_t *camera);

int camera_fill_rect(const camera_t *camera,
                     SDL_Renderer *render,
                     const rect_t *rect);

void camera_center_at(camera_t *camera, point_t position);

void camera_toggle_debug_mode(camera_t *camera);
void camera_zoom_in(camera_t * camera);
void camera_zoom_out(camera_t * camera);

#endif  // CAMERA_H_
