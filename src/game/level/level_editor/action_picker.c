#include <string.h>

#include "system/stacktrace.h"
#include "action_picker.h"
#include "math/extrema.h"
#include "math/vec.h"

static const char *action_labels[ACTION_N] = {
    [ACTION_NONE]        = "None",
    [ACTION_HIDE_LABEL]  = "Hide Label",
    [ACTION_TOGGLE_GOAL] = "Toggle Goal"
};

#define TEXT_SCALE vec(5.0f, 5.0f)
#define WIDGET_PADDING 30.0f
#define ELEMENT_WIDTH ((float)(longest_label() * FONT_CHAR_WIDTH) * TEXT_SCALE.x)
#define ELEMENT_HEIGHT (FONT_CHAR_HEIGHT * TEXT_SCALE.y)
#define WIDGET_WIDTH (ELEMENT_WIDTH + 2 * WIDGET_PADDING)
#define WIDGET_HEIGHT (ACTION_N * ELEMENT_HEIGHT + WIDGET_PADDING * (ACTION_N + 1))

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
                          const Camera *camera)
{
    trace_assert(picker);
    trace_assert(camera);
    (void) action_labels;

    camera_fill_rect_screen(
        camera,
        rect_from_vecs(
            picker->position,
            vec(WIDGET_WIDTH,WIDGET_HEIGHT)),
        COLOR_BLACK);

    for (size_t i = 0; i < ACTION_N; ++i) {
        const Vec2f element_position =
            vec_sum(
                picker->position,
                vec(
                    WIDGET_PADDING,
                    WIDGET_PADDING + (float)i * (ELEMENT_HEIGHT + WIDGET_PADDING)));

        camera_render_text_screen(
            camera,
            action_labels[i],
            TEXT_SCALE,
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

    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
            const Vec2f mouse_position =
                vec((float)event->button.x,
                    (float)event->button.y);


            for (size_t i = 0; i < ACTION_N; ++i) {
                const Rect element_box =
                    rect_from_vecs(
                        vec_sum(
                            action_picker->position,
                            vec(
                                WIDGET_PADDING,
                                WIDGET_PADDING + (float) i * (ELEMENT_HEIGHT + WIDGET_PADDING))),
                        vec(ELEMENT_WIDTH, ELEMENT_HEIGHT));

                if (rect_contains_point(element_box, mouse_position)) {
                    action_picker->action.type = i;
                    break;
                }
            }
        } break;
        }
    } break;
    }
}
