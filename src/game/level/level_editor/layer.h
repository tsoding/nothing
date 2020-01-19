#ifndef LAYER_H_
#define LAYER_H_

#include "game/camera.h"
#include "undo_history.h"

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

typedef struct Game Game;

int layer_render(LayerPtr layer, const Camera *camera, int active);
int layer_event(LayerPtr layer,
                const SDL_Event *event,
                const Camera *camera,
                UndoHistory *undo_history);
int layer_dump_stream(LayerPtr layer, FILE *stream);

#endif  // LAYER_H_
