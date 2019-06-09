#ifndef LABEL_LAYER_H_
#define LABEL_LAYER_H_

#include "layer.h"

typedef struct LabelLayer LabelLayer;
typedef struct Camera Camera;
typedef struct LineStream LineStream;

LayerPtr label_layer_as_layer(LabelLayer *label_layer);

LabelLayer *create_label_layer(void);
LabelLayer *create_label_layer_from_line_stream(LineStream *line_stream);
void destroy_label_layer(LabelLayer *label_layer);

int label_layer_render(const LabelLayer *label_layer,
                       Camera *camera,
                       float fa);
int label_layer_event(LabelLayer *label_layer,
                      const SDL_Event *event,
                      const Camera *camera);

#endif  // LABEL_LAYER_H_
