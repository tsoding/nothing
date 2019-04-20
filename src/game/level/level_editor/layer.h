#ifndef LAYER_H_
#define LAYER_H_

typedef struct Layer Layer;

Layer *create_layer(void);
void destroy_layer(Layer *layer);

int layer_render(const Layer *layer, Camera *camera);
int layer_event(Layer *layer, const SDL_Event *event);

int layer_add_rect(Layer *layer, Rect rect, Color color);
int layer_delete_rect_at(Layer *layer, Vec position);

#endif  // LAYER_H_
