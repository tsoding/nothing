#ifndef LAYER_H_
#define LAYER_H_

typedef enum {
    LAYER_RECT,
    LAYER_POINT,
    LAYER_PLAYER,
    LAYER_BACKGROUND,
    LAYER_LABEL
} LayerType;

typedef struct {
    LayerType type;
    void *ptr;
} LayerPtr;

typedef struct Camera Camera;
typedef struct UndoHistory UndoHistory;

int layer_render(LayerPtr layer, Camera *camera, int active);
int layer_event(LayerPtr layer,
                const SDL_Event *event,
                const Camera *camera,
                UndoHistory *undo_history);
int layer_dump_stream(LayerPtr layer, FILE *stream);

#endif  // LAYER_H_
