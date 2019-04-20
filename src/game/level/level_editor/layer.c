#include "game/camera.h"
#include "system/lt.h"
#include "system/stacktrace.h"
#include "system/nth_alloc.h"
#include "system/log.h"
#include "math/rect.h"
#include "color.h"
#include "layer.h"
#include "dynarray.h"

struct Layer {
    Lt *lt;
    Dynarray *rects;
    Dynarray *colors;
};

Layer *create_layer(void)
{
    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    Layer *layer = PUSH_LT(lt, nth_alloc(sizeof(Layer)), free);
    if (layer == NULL) {
        RETURN_LT(lt, NULL);
    }
    layer->lt = lt;

    layer->rects = PUSH_LT(
        lt,
        create_dynarray(sizeof(Rect)),
        destroy_dynarray);
    if (layer->rects == NULL) {
        RETURN_LT(lt, NULL);
    }

    layer->colors = PUSH_LT(
        lt,
        create_dynarray(sizeof(Color)),
        destroy_dynarray);
    if (layer->colors == NULL) {
        RETURN_LT(lt, NULL);
    }

    trace_assert(layer);

    return layer;
}

void destroy_layer(Layer *layer)
{
    trace_assert(layer);
    RETURN_LT0(layer->lt);
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

// TODO: layer_add_rect is not implemented
int layer_add_rect(Layer *layer, Rect rect, Color color)
{
    trace_assert(layer);
    (void) rect;
    (void) color;

    return 0;
}

// TODO: layer_delete_rect_at is not implemented
int layer_delete_rect_at(Layer *layer, Vec position)
{
    trace_assert(layer);
    (void) position;

    return 0;
}
