#ifndef LAYER_H_
#define LAYER_H_

// TODO(#836): RectLayer should be called RectLayer
typedef struct RectLayer RectLayer;
typedef struct LineStream LineStream;

RectLayer *create_layer(void);
RectLayer *create_layer_from_line_stream(LineStream *line_stream);
void destroy_layer(RectLayer *layer);

int layer_render(const RectLayer *layer, Camera *camera);
int layer_event(RectLayer *layer, const SDL_Event *event);

int layer_add_rect(RectLayer *layer, Rect rect, Color color);
int layer_delete_rect_at(RectLayer *layer, Vec position);

size_t layer_count(const RectLayer *layer);
const Rect *layer_rects(const RectLayer *layer);
const Color *layer_colors(const RectLayer *layer);

#endif  // LAYER_H_
