#include <SDL2/SDL.h>

#include "system/stacktrace.h"
#include "game/camera.h"

#include "./proto_rect.h"

ProtoRect *create_proto_rect(void)
{
    return NULL;
}

void destroy_proto_rect(ProtoRect *proto_rect)
{
    trace_assert(proto_rect);
}

int proto_rect_render(const ProtoRect *proto_rect,
                      Camera *camera)
{
    trace_assert(proto_rect);
    trace_assert(camera);
    return 0;
}

int proto_rect_update(ProtoRect *proto_rect,
                      float delta_time)
{
    trace_assert(proto_rect);
    (void) delta_time;
    return 0;
}

int proto_rect_events(ProtoRect *proto_rect,
                      SDL_Event *event)
{
    trace_assert(proto_rect);
    trace_assert(event);

    return 0;
}
