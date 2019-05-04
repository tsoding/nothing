#ifndef LAYER_H_
#define LAYER_H_

// TODO: Layer should be called RectLayer
typedef struct Layer Layer;
typedef struct LineStream LineStream;

Layer *create_layer(void);
Layer *create_layer_from_line_stream(LineStream *line_stream);
void destroy_layer(Layer *layer);

int layer_render(const Layer *layer, Camera *camera);
int layer_event(Layer *layer, const SDL_Event *event);

int layer_add_rect(Layer *layer, Rect rect, Color color);
int layer_delete_rect_at(Layer *layer, Vec position);

size_t layer_count(const Layer *layer);
const Rect *layer_rects(const Layer *layer);
const Color *layer_colors(const Layer *layer);

#endif  // LAYER_H_
