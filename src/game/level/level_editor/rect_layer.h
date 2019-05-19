#ifndef RECT_LAYER_H_
#define RECT_LAYER_H_

#include "layer.h"

typedef struct RectLayer RectLayer;
typedef struct LineStream LineStream;

LayerPtr rect_layer_as_layer(RectLayer *layer);
RectLayer *create_rect_layer(void);
RectLayer *create_rect_layer_from_line_stream(LineStream *line_stream);
void destroy_rect_layer(RectLayer *layer);

int rect_layer_render(const RectLayer *layer, Camera *camera);
int rect_layer_event(RectLayer *layer, const SDL_Event *event, const Camera *camera);

int rect_layer_add_rect(RectLayer *layer, Rect rect, Color color);
int rect_layer_delete_rect_at(RectLayer *layer, Vec position);

size_t rect_layer_count(const RectLayer *layer);
const Rect *rect_layer_rects(const RectLayer *layer);
const Color *rect_layer_colors(const RectLayer *layer);

#endif  // RECT_LAYER_H_
