#ifndef CAMERA_H_
#define CAMERA_H_

#include "./rect.h"
#include "./point.h"

typedef struct camera_t camera_t;

camera_t *create_camera();
void destroy_camera(camera_t *camera);

int camera_fill_rect(const camera_t *camera,
                     SDL_Renderer *render,
                     const rect_t *rect);

void camera_center_at(camera_t *camera, point_t position);

#endif  // CAMERA_H_
