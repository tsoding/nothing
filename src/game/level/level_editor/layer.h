#ifndef LAYER_H_
#define LAYER_H_

typedef enum {
    LAYER_RECT,
    LAYER_POINT,
    LAYER_PLAYER
} LayerType;

typedef struct {
    LayerType type;
    void *ptr;
} LayerPtr;

typedef struct Camera Camera;

int layer_render(LayerPtr layer, Camera *camera);
int layer_event(LayerPtr layer, const SDL_Event *event, const Camera *camera);

#endif  // LAYER_H_
