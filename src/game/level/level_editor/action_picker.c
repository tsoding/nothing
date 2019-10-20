#include "system/stacktrace.h"
#include "action_picker.h"

static const char *action_labels[ACTION_N] = {
    [ACTION_NONE]        = "None",
    [ACTION_HIDE_LABEL]  = "Hide Label",
    [ACTION_TOGGLE_GOAL] = "Toggle Goal"
};

#define ACTION_PICKER_PADDING 20.0f

void action_picker_render(const ActionPicker *picker,
                          const Camera *camera,
                          Vec2f position)
{
    trace_assert(picker);
    trace_assert(camera);
    (void) action_labels;

    Vec2f text_scale = vec(5.0f, 5.0f);

    camera_fill_rect_screen(
        camera,
        rect_pad(
            camera_text_boundary_box(
                camera,
                position,
                text_scale,
                action_labels[picker->action.type]),
            ACTION_PICKER_PADDING),
        COLOR_BLACK);

    camera_render_text_screen(
        camera,
        action_labels[picker->action.type],
        text_scale,
        COLOR_RED,
        position);
}

void action_picker_event(ActionPicker *action_picker,
                         const SDL_Event *event)
{
    trace_assert(action_picker);
    trace_assert(event);
}
