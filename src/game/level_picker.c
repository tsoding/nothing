#include <stdio.h>

#include "./level_picker.h"
#include "game/level/background.h"
#include "game/sprite_font.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/stacktrace.h"
#include "system/str.h"
#include "ui/list_selector.h"
#include "system/log.h"
#include "game/level_folder.h"
#include "ui/wiggly_text.h"
#include "ui/slider.h"

#define TITLE_MARGIN_TOP 100.0f
#define TITLE_MARGIN_BOTTOM 100.0f

struct LevelPicker
{
    Lt *lt;
    Background background;
    Vec2f camera_position;
    LevelFolder level_folder;
    WigglyText wiggly_text;
    ListSelector list_selector;
};

static inline
const char *list_item_text(void *element)
{
    trace_assert(element);
    return ((LevelMetadata *)element)->title;
}

LevelPicker *create_level_picker(const char *dirpath)
{
    trace_assert(dirpath);

    Lt *lt = create_lt();

    LevelPicker *level_picker = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(LevelPicker)),
        free);
    if (level_picker == NULL) {
        RETURN_LT(lt, NULL);
    }
    level_picker->lt = lt;

    level_picker->background = create_background(hexstr("073642"));

    level_picker->camera_position = vec(0.0f, 0.0f);

    level_picker->level_folder = create_level_folder();
    level_folder_read("./assets/levels", &level_picker->level_folder);

    level_picker->wiggly_text = (WigglyText) {
        .text = "Select Level",
        .scale = {10.0f, 10.0f},
        .color = COLOR_WHITE,
    };

    level_picker->list_selector.items = level_picker->level_folder.metadatas;
    level_picker->list_selector.font_scale = vec(5.0f, 5.0f);
    level_picker->list_selector.padding_bottom = 50.0f;
    level_picker->list_selector.list_item_text = list_item_text;

    return level_picker;
}

void destroy_level_picker(LevelPicker *level_picker)
{
    trace_assert(level_picker);
    destroy_level_folder(level_picker->level_folder);
    RETURN_LT0(level_picker->lt);
}

int level_picker_render(const LevelPicker *level_picker,
                        const Camera *camera)
{
    trace_assert(level_picker);

    const Rect viewport = camera_view_port_screen(camera);

    if (background_render(&level_picker->background, camera) < 0) {
        return -1;
    }

    const Vec2f title_size = wiggly_text_size(&level_picker->wiggly_text);

    wiggly_text_render(
        &level_picker->wiggly_text,
        camera,
        vec(viewport.w * 0.5f - title_size.x * 0.5f, TITLE_MARGIN_TOP));

    list_selector_render(camera, &level_picker->list_selector);

    {
        /* CSS */
        const float padding = 20.0f;
        const Vec2f size = vec(3.0f, 3.0f);
        const Vec2f position = vec(0.0f, viewport.h - size.y * FONT_CHAR_HEIGHT);

        /* HTML */
        camera_render_text_screen(
            camera,
            "Press 'N' to create new level",
            size,
            COLOR_WHITE,
            vec(position.x + padding,
                position.y - padding));
    }

    return 0;
}

int level_picker_update(LevelPicker *level_picker,
                        float delta_time)
{
    trace_assert(level_picker);

    vec_add(&level_picker->camera_position,
            vec(50.0f * delta_time, 0.0f));

    if (wiggly_text_update(&level_picker->wiggly_text, delta_time) < 0) {
        return -1;
    }

    return 0;
}

int level_picker_event(LevelPicker *level_picker,
                       const SDL_Event *event)
{
    trace_assert(level_picker);
    trace_assert(event);

    switch (event->type) {
    case SDL_WINDOWEVENT: {
        switch (event->window.event) {
        case SDL_WINDOWEVENT_SHOWN:
        case SDL_WINDOWEVENT_SIZE_CHANGED: {
            const Vec2f font_scale = vec(5.0f, 5.0f);
            const float padding_bottom = 50.0f;

            int width;
            SDL_GetRendererOutputSize(SDL_GetRenderer(SDL_GetWindowFromID(event->window.windowID)), &width, NULL);

            const Vec2f title_size = wiggly_text_size(&level_picker->wiggly_text);

            const Vec2f selector_size = list_selector_size(
                &level_picker->list_selector,
                font_scale,
                padding_bottom);

            level_picker->list_selector.position =
                vec((float)width * 0.5f - selector_size.x * 0.5f,
                    TITLE_MARGIN_TOP + title_size.y + TITLE_MARGIN_BOTTOM);
        } break;
        }
    } break;
    }

    return list_selector_event(&level_picker->list_selector, event);
}

int level_picker_input(LevelPicker *level_picker,
                       const Uint8 *const keyboard_state,
                       SDL_Joystick *the_stick_of_joy)
{
    trace_assert(level_picker);
    trace_assert(keyboard_state);
    (void) the_stick_of_joy;
    return 0;
}

const char *level_picker_selected_level(const LevelPicker *level_picker)
{
    trace_assert(level_picker);

    if (level_picker->list_selector.selected_item < 0) {
        return NULL;
    }

    LevelMetadata *metadata = dynarray_pointer_at(
        &level_picker->level_folder.metadatas,
        (size_t)level_picker->list_selector.selected_item);

    return metadata->filepath;
}

void level_picker_clean_selection(LevelPicker *level_picker)
{
    trace_assert(level_picker);
    level_picker->list_selector.selected_item = -1;
}

int level_picker_enter_camera_event(LevelPicker *level_picker,
                                    Camera *camera)
{
    camera_center_at(camera, level_picker->camera_position);
    return 0;
}
