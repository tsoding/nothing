#ifndef LABELS_H_
#define LABELS_H_

#include "math/vec.h"
#include "color.h"
#include "config.h"
#include "game/level/level_editor/label_layer.h"

typedef struct Labels Labels;

Labels *create_labels_from_label_layer(const LabelLayer *label_layer);
void destroy_labels(Labels *label);

int labels_render(const Labels *label,
                  const Camera *camera);
void labels_update(Labels *label,
                   float delta_time);
void labels_enter_camera_event(Labels *label,
                               const Camera *camera);
void labels_hide(Labels *label, char id[ENTITY_MAX_ID_SIZE]);

#endif  // LABELS_H_
