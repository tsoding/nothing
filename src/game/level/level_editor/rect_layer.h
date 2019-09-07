#ifndef RECT_LAYER_H_
#define RECT_LAYER_H_

#include "layer.h"

#define RECT_LAYER_ID_MAX_SIZE 36

typedef struct RectLayer RectLayer;
typedef struct LineStream LineStream;

LayerPtr rect_layer_as_layer(RectLayer *layer);
RectLayer *create_rect_layer(void);
RectLayer *create_rect_layer_from_line_stream(LineStream *line_stream);
void destroy_rect_layer(RectLayer *layer);

int rect_layer_render(const RectLayer *layer, Camera *camera, int active);
int rect_layer_event(RectLayer *layer,
                     const SDL_Event *event,
                     const Camera *camera,
                     UndoHistory *undo_history);

int rect_layer_dump_stream(const RectLayer *layer, FILE *filedump);

size_t rect_layer_count(const RectLayer *layer);
const Rect *rect_layer_rects(const RectLayer *layer);
const Color *rect_layer_colors(const RectLayer *layer);
const char *rect_layer_ids(const RectLayer *layer);

#endif  // RECT_LAYER_H_
