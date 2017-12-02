#ifndef CAMERA_H_
#define CAMERA_H_

#include "./rect.h"
#include "./point.h"

typedef struct camera camera_t;

camera_t *create_camera();
void destroy_camera(camera_t *camera);

int camera_fill_rect(const camera_t *camera,
                     SDL_Renderer *render,
                     const struct rect_t *rect);

void camera_translate(camera_t *camera, float x, float y);

// TODO(#16): implement camera_center_at
//
// Camera should encapsulate size of the screen and use it to center
// itself at a certain position

#endif  // CAMERA_H_
