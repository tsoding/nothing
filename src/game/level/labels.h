#ifndef LABELS_H_
#define LABELS_H_

#include "math/vec.h"
#include "color.h"
#include "ebisp/expr.h"

typedef struct Labels Labels;
typedef struct LineStream LineStream;
typedef struct LabelLayer LabelLayer;

Labels *create_labels_from_line_stream(LineStream *line_stream);
Labels *create_labels_from_label_layer(const LabelLayer *label_layer);
void destroy_labels(Labels *label);

int labels_render(const Labels *label,
                  const Camera *camera);
void labels_update(Labels *label,
                   float delta_time);
void labels_enter_camera_event(Labels *label,
                               const Camera *camera);

struct EvalResult
labels_send(Labels *labels, Gc *gc, struct Scope *scope, struct Expr path);

#endif  // LABELS_H_
