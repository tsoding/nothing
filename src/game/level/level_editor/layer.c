#include "game/camera.h"
#include "system/lt.h"
#include "system/stacktrace.h"
#include "layer.h"
#include "dynarray.h"

struct Layer {
    Lt *lt;
    Dynarray *rects;
    Dynarray *colors;
};

Layer *create_layer(void)
{
    return NULL;
}

void destroy_layer(Layer *layer)
{
    trace_assert(layer);
}

int layer_render(const Layer *layer, Camera *camera)
{
    trace_assert(layer);
    trace_assert(camera);
    return 0;
}

int layer_event(Layer *layer, const SDL_Event *event)
{
    trace_assert(layer);
    trace_assert(event);
    return 0;
}
