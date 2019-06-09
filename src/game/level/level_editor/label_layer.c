#include <SDL.h>

#include "system/line_stream.h"
#include "system/stacktrace.h"
#include "system/nth_alloc.h"
#include "system/lt.h"
#include "label_layer.h"

struct LabelLayer {
    Lt *lt;
};

LayerPtr label_layer_as_layer(LabelLayer *label_layer)
{
    LayerPtr layer = {
        .ptr = label_layer,
        .type = LAYER_LABEL
    };
    return layer;
}

LabelLayer *create_label_layer(void)
{
    return nth_calloc(1, sizeof(LabelLayer));
}

LabelLayer *create_label_layer_from_line_stream(LineStream *line_stream)
{
    trace_assert(line_stream);
    return nth_calloc(1, sizeof(LabelLayer));
}

void destroy_label_layer(LabelLayer *label_layer)
{
    trace_assert(label_layer);
    free(label_layer);
}

int label_layer_render(const LabelLayer *label_layer,
                       Camera *camera,
                       float fa)
{
    trace_assert(label_layer);
    trace_assert(camera);
    (void) fa;
    return 0;
}

int label_layer_event(LabelLayer *label_layer,
                      const SDL_Event *event,
                      const Camera *camera)
{
    trace_assert(label_layer);
    trace_assert(event);
    trace_assert(camera);
    return 0;
}
