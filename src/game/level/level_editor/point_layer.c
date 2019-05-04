#include <SDL2/SDL.h>

#include "system/stacktrace.h"
#include "system/line_stream.h"
#include "game/camera.h"
#include "./point_layer.h"

PointLayer *create_point_layer(void)
{
    return NULL;
}

PointLayer *create_point_layer_from_line_stream(LineStream *line_stream)
{
    trace_assert(line_stream);
    return NULL;
}

void destroy_point_layer(PointLayer *point_layer)
{
    trace_assert(point_layer);
}

int point_layer_render(const PointLayer *point_layer,
                       Camera *camera)
{
    trace_assert(point_layer);
    trace_assert(camera);

    return 0;
}

int point_layer_event(PointLayer *point_layer,
                      const SDL_Event *event)
{
    trace_assert(point_layer);
    trace_assert(event);
    return 0;
}
