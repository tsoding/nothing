#include <stdbool.h>

#include <SDL2/SDL.h>

#include "system/stacktrace.h"
#include "system/log.h"
#include "game/level/boxes.h"
#include "game/camera.h"

#include "./proto_rect.h"

int proto_rect_render(const ProtoRect *proto_rect,
                      Camera *camera)
{
    trace_assert(proto_rect);
    trace_assert(camera);

    if (proto_rect->active) {
        if (camera_fill_rect(
                camera,
                rect_from_points(
                    proto_rect->begin,
                    proto_rect->end),
                proto_rect->color) < 0) {
            return -1;
        }
    }

    return 0;
}

int proto_rect_update(ProtoRect *proto_rect,
                      float delta_time)
{
    trace_assert(proto_rect);
    (void) delta_time;
    return 0;
}

int proto_rect_event(ProtoRect *proto_rect,
                     const SDL_Event *event,
                     const Camera *camera,
                     Boxes *boxes)
{
    trace_assert(proto_rect);
    trace_assert(event);
    trace_assert(camera);
    trace_assert(boxes);

    if (proto_rect->active) {
        // Active
        switch (event->type) {
        case SDL_MOUSEBUTTONUP: {
            switch (event->button.button) {
            case SDL_BUTTON_LEFT: {
                boxes_add_box(
                    boxes,
                    rect_from_points(
                        proto_rect->begin,
                        proto_rect->end),
                    proto_rect->color);
                proto_rect->active = false;
            } break;
            }
        } break;

        case SDL_MOUSEMOTION: {
            proto_rect->end = camera_map_screen(
                camera,
                event->motion.x,
                event->motion.y);
        } break;
        }
    } else {
        // Inactive
        switch (event->type) {
        case SDL_MOUSEBUTTONDOWN: {
            switch (event->button.button) {
            case SDL_BUTTON_LEFT: {
                proto_rect->active = true;
                proto_rect->begin = camera_map_screen(
                    camera,
                    event->button.x,
                    event->button.y);
                proto_rect->end = proto_rect->begin;
            } break;
            }
        } break;
        }
    }

    return 0;
}
