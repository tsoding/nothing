#include <stdbool.h>

#include "game/camera.h"
#include "game/level/boxes.h"
#include "game/level/proto_rect.h"
#include "system/stacktrace.h"
#include "system/nth_alloc.h"

#include "level_editor.h"

struct LevelEditor
{
    Vec camera_position;
    float camera_scale;
    ProtoRect proto_rect;
    Boxes *boxes;
};

LevelEditor *create_level_editor(Boxes *boxes)
{
    LevelEditor *level_editor = nth_calloc(1, sizeof(LevelEditor));
    level_editor->camera_position = vec(0.0f, 0.0f);
    level_editor->camera_scale = 1.0f;
    level_editor->boxes = boxes;
    level_editor->proto_rect.color = rgba(1.0f, 0.0f, 0.0f, 1.0f);

    return level_editor;
}

void destroy_level_editor(LevelEditor *level_editor)
{
    trace_assert(level_editor);
    free(level_editor);
}

int level_editor_render(const LevelEditor *level_editor,
                        Camera *camera)
{
    trace_assert(level_editor);
    trace_assert(camera);

    if (proto_rect_render(&level_editor->proto_rect, camera) < 0) {
        return -1;
    }

    return 0;
}

int level_editor_update(LevelEditor *level_editor,
                        float delta_time)
{
    trace_assert(level_editor);

    if (proto_rect_update(&level_editor->proto_rect, delta_time) < 0) {
        return -1;
    }

    return 0;
}

int level_editor_event(LevelEditor *level_editor,
                       const SDL_Event *event,
                       const Camera *camera)
{
    trace_assert(level_editor);
    trace_assert(event);

    switch (event->type) {
    case SDL_MOUSEMOTION: {
        const float sens = 1.0f / level_editor->camera_scale * 0.25f;
        vec_add(&level_editor->camera_position,
                vec((float) event->motion.xrel * sens, (float) event->motion.yrel * sens));
    } break;

    case SDL_MOUSEWHEEL: {
        // TODO(#679): zooming in edit mode is not smooth enough
        if (event->wheel.y > 0) {
            level_editor->camera_scale += 0.1f;
        } else if (event->wheel.y < 0) {
            level_editor->camera_scale = fmaxf(0.1f, level_editor->camera_scale - 0.1f);
        }
    } break;
    }

    if (proto_rect_event(
            &level_editor->proto_rect,
            event,
            camera,
            level_editor->boxes) < 0) {
        return -1;
    }

    return 0;
}

int level_editor_focus_camera(LevelEditor *level_editor,
                              Camera *camera)
{
    camera_center_at(camera, level_editor->camera_position);
    camera_scale(camera, level_editor->camera_scale);
    return 0;
}
