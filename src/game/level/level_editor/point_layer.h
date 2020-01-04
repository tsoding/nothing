#ifndef POINT_LAYER_H_
#define POINT_LAYER_H_

#include "math/vec.h"
#include "color.h"
#include "layer.h"

#define ID_MAX_SIZE 36

typedef struct PointLayer PointLayer;

LayerPtr point_layer_as_layer(PointLayer *point_layer);
// NOTE: create_point_layer and create_point_layer_from_line_stream do
// not own id_name_prefix
PointLayer *create_point_layer(const char *id_name_prefix);
PointLayer *chop_point_layer(Memory *memory,
                             String *input,
                             const char *id_name_prefix);
void destroy_point_layer(PointLayer *point_layer);

int point_layer_render(const PointLayer *point_layer,
                       const Camera *camera,
                       int active);
int point_layer_event(PointLayer *point_layer,
                      const SDL_Event *event,
                      const Camera *camera,
                      UndoHistory *undo_history);

int point_layer_dump_stream(const PointLayer *point_layer,
                            FILE *filedump);

size_t point_layer_count(const PointLayer *point_layer);
const Vec2f *point_layer_positions(const PointLayer *point_layer);
const Color *point_layer_colors(const PointLayer *point_layer);
const char *point_layer_ids(const PointLayer *point_layer);

#endif  // POINT_LAYER_H_
