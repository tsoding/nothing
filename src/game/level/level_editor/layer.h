#ifndef LAYER_H_
#define LAYER_H_

typedef struct Layer Layer;

Layer *create_layer(void);
void destroy_layer(Layer *layer);

int layer_render(const Layer *layer, Camera *camera);
int layer_event(Layer *layer, const SDL_Event *event);

#endif  // LAYER_H_
