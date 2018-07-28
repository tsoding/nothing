#ifndef LABELS_H_
#define LABELS_H_

#include "math/point.h"
#include "color.h"

typedef struct labels_t labels_t;
typedef struct camera_t camera_t;

labels_t *create_labels_from_stream(FILE *stream);
void destroy_labels(labels_t *label);

int labels_render(const labels_t *label,
                  camera_t *camera);
void labels_update(labels_t *label,
                   float delta_time);
void labels_enter_camera_event(labels_t *label,
                               const camera_t *camera);

#endif  // LABELS_H_
