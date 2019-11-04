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
#define TEXT_COLOR COLOR_WHITE
#define SELECTION_COLOR COLOR_WHITE
#define BACKGROUND_COLOR COLOR_BLACK

void action_picker_render(const ActionPicker *picker,
                          const Camera *camera)
{
    trace_assert(picker);
    trace_assert(camera);
    (void) action_labels;

    camera_fill_rect_screen(
        camera,
        picker->widget.boundary,
        BACKGROUND_COLOR);

    const float element_height = picker->widget.boundary.h / (float)ACTION_N;
    for (size_t i = 0; i < ACTION_N; ++i) {
        const Vec2f element_position =
            vec_sum(
                vec(picker->widget.boundary.x, picker->widget.boundary.y),
                vec(0.0f, (float)i * element_height));
        const Rect element_box =
            rect_from_vecs(element_position,
                           vec(picker->widget.boundary.w, element_height));


        camera_render_text_screen(
            camera,
            action_labels[i],
            TEXT_SCALE,
            TEXT_COLOR,
            element_position);

        if (i == picker->action.type) {
            camera_draw_thicc_rect_screen(
                camera,
                element_box,
                SELECTION_COLOR,
                5.0f);
        }
    }
}

void action_picker_event(ActionPicker *picker,
                         const SDL_Event *event)
{
    trace_assert(picker);
    trace_assert(event);

    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
            const Vec2f mouse_position =
                vec((float)event->button.x,
                    (float)event->button.y);

            const float element_height = picker->widget.boundary.h / (float)ACTION_N;

            for (ActionType i = 0; i < ACTION_N; ++i) {
                const Vec2f element_position =
                    vec_sum(
                        vec(picker->widget.boundary.x, picker->widget.boundary.y),
                        vec(0.0f, (float)i * element_height));
                const Rect element_box =
                    rect_from_vecs(element_position,
                                   vec(picker->widget.boundary.w, element_height));

                if (rect_contains_point(element_box, mouse_position)) {
                    picker->action.type = i;
                    break;
                }
            }
        } break;
        }
    } break;

    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_UP: {
            if (picker->action.type > 0) {
                picker->action.type--;
            }
        } break;

        case SDLK_DOWN: {
            if (picker->action.type < ACTION_N) {
                picker->action.type++;
            }
        } break;
        }
    } break;
    }
}
