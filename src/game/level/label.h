#ifndef LABEL_H_
#define LABEL_H_

#include "math/point.h"
#include "color.h"

typedef struct label_t label_t;
typedef struct camera_t camera_t;

label_t *create_label(vec_t position,
                      color_t color,
                      const char *text);
void destroy_label(label_t *label);

int label_render(const label_t *label,
                 camera_t *camera);
void label_update(label_t *label,
                  float delta_time);

#endif  // LABEL_H_
