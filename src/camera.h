#ifndef CAMERA_H_
#define CAMERA_H_

#include "./rect.h"
#include "./point.h"

typedef struct {
    point_t translation;
} camera_t;

camera_t *create_camera();
void destroy_camera(camera_t *camera);

int camera_fill_rect(const camera_t *camera,
                     SDL_Renderer *render,
                     const struct rect_t *rect);

#endif  // CAMERA_H_
