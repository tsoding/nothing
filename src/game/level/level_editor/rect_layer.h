#ifndef LAYER_H_
#define LAYER_H_

typedef struct RectLayer RectLayer;
typedef struct LineStream LineStream;

RectLayer *create_rect_layer(void);
RectLayer *create_rect_layer_from_line_stream(LineStream *line_stream);
void destroy_rect_layer(RectLayer *layer);

int rect_layer_render(const RectLayer *layer, Camera *camera);
int rect_layer_event(RectLayer *layer, const SDL_Event *event);

int rect_layer_add_rect(RectLayer *layer, Rect rect, Color color);
int rect_layer_delete_rect_at(RectLayer *layer, Vec position);

size_t rect_layer_count(const RectLayer *layer);
const Rect *rect_layer_rects(const RectLayer *layer);
const Color *rect_layer_colors(const RectLayer *layer);

#endif  // LAYER_H_
