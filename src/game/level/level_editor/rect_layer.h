#ifndef RECT_LAYER_H_
#define RECT_LAYER_H_

#include "layer.h"
#include "game/level/action.h"
#include "ui/cursor.h"

typedef struct RectLayer RectLayer;
typedef struct LineStream LineStream;

LayerPtr rect_layer_as_layer(RectLayer *layer);
// NOTE: create_rect_layer and create_rect_layer_from_line_stream does
// not own id_name_prefix
RectLayer *create_rect_layer(const char *id_name_prefix,
                             Cursor *cursor);
RectLayer *create_rect_layer_from_line_stream(
    LineStream *line_stream,
    const char *id_name_prefix,
    Cursor *cursor);
void destroy_rect_layer(RectLayer *layer);

int rect_layer_render(const RectLayer *layer, const Camera *camera, int active);
int rect_layer_event(RectLayer *layer,
                     const SDL_Event *event,
                     const Camera *camera,
                     UndoHistory *undo_history);

int rect_layer_dump_stream(const RectLayer *layer, FILE *filedump);

size_t rect_layer_count(const RectLayer *layer);
const Rect *rect_layer_rects(const RectLayer *layer);
const Color *rect_layer_colors(const RectLayer *layer);
const char *rect_layer_ids(const RectLayer *layer);
const Action *rect_layer_actions(const RectLayer *layer);

#endif  // RECT_LAYER_H_
