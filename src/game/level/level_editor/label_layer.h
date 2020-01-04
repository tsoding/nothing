#ifndef LABEL_LAYER_H_
#define LABEL_LAYER_H_

#include "layer.h"
#include "color.h"
#include "math/vec.h"

#define LABELS_SIZE vec(2.0f, 2.0f)
#define LABEL_LAYER_ID_MAX_SIZE 36
#define LABEL_LAYER_TEXT_MAX_SIZE 256

typedef struct LabelLayer LabelLayer;
typedef struct LineStream LineStream;

LayerPtr label_layer_as_layer(LabelLayer *label_layer);

// NOTE: create_label_layer and create_label_layer_from_line_stream do
// not own id_name_prefix
LabelLayer *create_label_layer(const char *id_name_prefix);
LabelLayer *create_label_layer_from_line_stream(LineStream *line_stream,
                                                const char *id_name_prefix);
LabelLayer *chop_label_layer(Memory *memory,
                             String *input,
                             const char *id_name_prefix);
void destroy_label_layer(LabelLayer *label_layer);

int label_layer_render(const LabelLayer *label_layer,
                       const Camera *camera,
                       int active);
int label_layer_event(LabelLayer *label_layer,
                      const SDL_Event *event,
                      const Camera *camera,
                      UndoHistory *undo_history);

size_t label_layer_count(const LabelLayer *label_layer);

int label_layer_dump_stream(const LabelLayer *label_layer, FILE *filedump);

char *label_layer_ids(const LabelLayer *label_layer);
Vec2f *label_layer_positions(const LabelLayer *label_layer);
Color *label_layer_colors(const LabelLayer *label_layer);
char *labels_layer_texts(const LabelLayer *label_layer);

#endif  // LABEL_LAYER_H_
