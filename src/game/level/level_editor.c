#include <stdbool.h>

#include "game/camera.h"
#include "game/level/boxes.h"
#include "game/level/level_editor/proto_rect.h"
#include "game/level/level_editor/color_picker.h"
#include "game/level/level_editor/rect_layer.h"
#include "game/level/level_editor/point_layer.h"
#include "game/level/level_editor/player_layer.h"
#include "game/level/level_editor/layer_picker.h"
#include "system/stacktrace.h"
#include "system/nth_alloc.h"
#include "system/lt.h"

#include "level_editor.h"

struct LevelEditor
{
    Lt *lt;
    Vec camera_position;
    float camera_scale;
    LayerPicker layer_picker;

    RectLayer *boxes_layer;
    RectLayer *platforms_layer;
    RectLayer *back_platforms_layer;
    PointLayer *goals_layer;
    PlayerLayer player_layer;
    LayerPtr layers[LAYER_PICKER_N];

    bool drag;
};

LevelEditor *create_level_editor(RectLayer *boxes_layer,
                                 RectLayer *platforms_layer,
                                 RectLayer *back_platforms_layer,
                                 PointLayer *goals_layer)
{
    trace_assert(boxes_layer);
    trace_assert(platforms_layer);
    trace_assert(back_platforms_layer);
    trace_assert(goals_layer);

    Lt *lt = create_lt();

    LevelEditor *level_editor = PUSH_LT(lt, nth_calloc(1, sizeof(LevelEditor)), free);
    if (level_editor == NULL) {
        RETURN_LT(lt, NULL);
    }
    level_editor->lt = lt;

    level_editor->camera_position = vec(0.0f, 0.0f);
    level_editor->camera_scale = 1.0f;

    level_editor->boxes_layer = PUSH_LT(lt, boxes_layer, destroy_rect_layer);
    level_editor->platforms_layer = PUSH_LT(lt, platforms_layer, destroy_rect_layer);
    level_editor->back_platforms_layer = PUSH_LT(lt, back_platforms_layer, destroy_rect_layer);
    level_editor->goals_layer = PUSH_LT(lt, goals_layer, destroy_point_layer);
    level_editor->player_layer.color_picker.color = rgba(0.0f, 0.0f, 0.0f, 1.0f);

    level_editor->layers[LAYER_PICKER_BOXES] = rect_layer_as_layer(level_editor->boxes_layer);
    level_editor->layers[LAYER_PICKER_PLATFORMS] = rect_layer_as_layer(level_editor->platforms_layer);
    level_editor->layers[LAYER_PICKER_BACK_PLATFORMS] = rect_layer_as_layer(level_editor->back_platforms_layer);
    level_editor->layers[LAYER_PICKER_GOALS] = point_layer_as_layer(level_editor->goals_layer);
    level_editor->layers[LAYER_PICKER_PLAYER] = player_layer_as_layer(&level_editor->player_layer);

    level_editor->layer_picker = LAYER_PICKER_BOXES;

    level_editor->drag = false;

    return level_editor;
}

void destroy_level_editor(LevelEditor *level_editor)
{
    trace_assert(level_editor);
    RETURN_LT0(level_editor->lt);
}

int level_editor_render(const LevelEditor *level_editor,
                        Camera *camera)
{
    trace_assert(level_editor);
    trace_assert(camera);

    for (size_t i = 0; i < LAYER_PICKER_N; ++i) {
        if (layer_render(
                level_editor->layers[i],
                camera,
                i == level_editor->layer_picker ? 1.0f : 0.5f) < 0) {
            return -1;
        }
    }

    if (layer_picker_render(&level_editor->layer_picker, camera) < 0) {
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
    trace_assert(camera);

    switch (event->type) {
    case SDL_MOUSEWHEEL: {
        // TODO(#679): zooming in edit mode is not smooth enough
        if (event->wheel.y > 0) {
            level_editor->camera_scale += 0.1f;
        } else if (event->wheel.y < 0) {
            level_editor->camera_scale = fmaxf(0.1f, level_editor->camera_scale - 0.1f);
        }
    } break;

    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN: {
        if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_MIDDLE) {
            level_editor->drag = true;
        }

        if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_MIDDLE) {
            level_editor->drag = false;
        }
    } break;

    case SDL_MOUSEMOTION: {
        if (level_editor->drag) {
            const Vec next_position = camera_map_screen(camera, event->motion.x, event->motion.y);
            const Vec prev_position = camera_map_screen(
                camera,
                event->motion.x + event->motion.xrel,
                event->motion.y + event->motion.yrel);

            vec_add(&level_editor->camera_position,
                    vec_sub(next_position, prev_position));
        }

    } break;
    }

    bool selected = false;
    if (layer_picker_event(
            &level_editor->layer_picker,
            event,
            camera,
            &selected) < 0) {
        return -1;
    }

    if (!selected) {
        if (layer_event(
                level_editor->layers[level_editor->layer_picker],
                event,
                camera) < 0) {
            return -1;
        }
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

const RectLayer *level_editor_boxes(const LevelEditor *level_editor)
{
    return level_editor->boxes_layer;
}

const RectLayer *level_editor_platforms(const LevelEditor *level_editor)
{
    return level_editor->platforms_layer;
}

const RectLayer *level_editor_back_platforms(const LevelEditor *level_editor)
{
    return level_editor->back_platforms_layer;
}

const PointLayer *level_editor_goals_layer(const LevelEditor *level_editor)
{
    return level_editor->goals_layer;
}
