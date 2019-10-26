#include <string.h>

#include "system/stacktrace.h"
#include "action_picker.h"
#include "math/extrema.h"

static const char *action_labels[ACTION_N] = {
    [ACTION_NONE]        = "None",
    [ACTION_HIDE_LABEL]  = "Hide Label",
    [ACTION_TOGGLE_GOAL] = "Toggle Goal"
};

static size_t longest_label(void)
{
    size_t result = 0;
    for (size_t i = 0; i < ACTION_N; ++i) {
        trace_assert(action_labels[i]);
        result = max_size_t(result, strlen(action_labels[i]));
    }
    return result;
}

void action_picker_render(const ActionPicker *picker,
                          const Camera *camera,
                          Vec2f position)
{
    trace_assert(picker);
    trace_assert(camera);
    (void) action_labels;

    Vec2f text_scale = vec(5.0f, 5.0f);

#define WIDGET_PADDING 30.0f
#define ELEMENT_WIDTH ((float)(longest_label() * FONT_CHAR_WIDTH) * text_scale.x)
#define ELEMENT_HEIGHT (FONT_CHAR_HEIGHT * text_scale.y)
#define WIDGET_WIDTH (ELEMENT_WIDTH + 2 * WIDGET_PADDING)
#define WIDGET_HEIGHT (ACTION_N * ELEMENT_HEIGHT + WIDGET_PADDING * (ACTION_N + 1))

    camera_fill_rect_screen(
        camera,
        rect_from_vecs(
            position,
            vec(WIDGET_WIDTH,WIDGET_HEIGHT)),
        COLOR_BLACK);

    for (size_t i = 0; i < ACTION_N; ++i) {
        const Vec2f element_position =
            vec_sum(
                position,
                vec(
                    WIDGET_PADDING,
                    WIDGET_PADDING + (float)i * (ELEMENT_HEIGHT + WIDGET_PADDING)));

        camera_render_text_screen(
            camera,
            action_labels[i],
            text_scale,
            COLOR_RED,
            element_position);

        if (i == picker->action.type) {
            camera_draw_thicc_rect_screen(
                camera,
                rect_from_vecs(
                    element_position,
                    vec(ELEMENT_WIDTH, ELEMENT_HEIGHT)),
                COLOR_RED,
                5.0f);
        }
    }
}

void action_picker_event(ActionPicker *action_picker,
                         const SDL_Event *event)
{
    trace_assert(action_picker);
    trace_assert(event);
}
