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
#include "ui/menu_title.h"

struct LevelPicker
{
    Lt *lt;
    Background *background;
    Vec camera_position;
    LevelFolder *level_folder;
    MenuTitle *menu_title;
    ListSelector *list_selector;
};

LevelPicker *create_level_picker(const Sprite_font *sprite_font, const char *dirpath)
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

    level_picker->background = PUSH_LT(
        lt,
        create_background(hexstr("073642")),
        destroy_background);
    if (level_picker->background == NULL) {
        RETURN_LT(lt, NULL);
    }

    level_picker->camera_position = vec(0.0f, 0.0f);

    level_picker->level_folder = PUSH_LT(
        lt,
        create_level_folder(dirpath),
        destroy_level_folder);
    if (level_picker->level_folder == NULL) {
        RETURN_LT(lt, NULL);
    }

    level_picker->menu_title = PUSH_LT(
        lt,
        create_menu_title("Select Level", vec(10.0f, 10.0f), sprite_font),
        destroy_menu_title);
    if (level_picker->menu_title == NULL) {
        RETURN_LT(lt, NULL);
    }

    level_picker->list_selector = PUSH_LT(
        lt,
        create_list_selector(
            sprite_font,
            level_folder_titles(level_picker->level_folder),
            level_folder_count(level_picker->level_folder),
            vec(5.0f, 5.0f),
            50.0f),
        destroy_list_selector);
    if (level_picker->list_selector == NULL) {
        RETURN_LT(lt, NULL);
    }

    return level_picker;
}

void destroy_level_picker(LevelPicker *level_picker)
{
    trace_assert(level_picker);
    RETURN_LT0(level_picker->lt);
}

int level_picker_render(const LevelPicker *level_picker,
                        Camera *camera,
                        SDL_Renderer *renderer)
{
    trace_assert(level_picker);
    trace_assert(renderer);

    if (background_render(level_picker->background, camera) < 0) {
        return -1;
    }

    if (menu_title_render(level_picker->menu_title, renderer) < 0) {
        return -1;
    }

    if (list_selector_render(level_picker->list_selector, renderer) < 0) {
        return -1;
    }

    return 0;
}

int level_picker_update(LevelPicker *level_picker,
                        float delta_time)
{
    trace_assert(level_picker);

    vec_add(&level_picker->camera_position,
            vec(50.0f * delta_time, 0.0f));

    if (menu_title_update(level_picker->menu_title, delta_time) < 0) {
        return -1;
    }

    return 0;
}

int level_picker_event(LevelPicker *level_picker, const SDL_Event *event)
{
    trace_assert(level_picker);
    trace_assert(event);

    switch (event->type) {
    case SDL_WINDOWEVENT: {
        switch (event->window.event) {
        case SDL_WINDOWEVENT_RESIZED: {
            const Vec font_scale = vec(5.0f, 5.0f);
            const float padding_bottom = 50.0f;

            const float width = (float) event->window.data1;

            const Vec title_size = menu_title_size(level_picker->menu_title);
            const float title_margin_top = 100.0f;
            const float title_margin_bottom = 100.0f;

            menu_title_move(
                level_picker->menu_title,
                vec(width * 0.5f - title_size.x * 0.5f, title_margin_top));

            const Vec selector_size = list_selector_size(
                level_picker->list_selector,
                font_scale,
                padding_bottom);

            list_selector_move(
                level_picker->list_selector,
                vec(width * 0.5f - selector_size.x * 0.5f,
                    title_margin_top + title_size.y + title_margin_bottom));
        } break;
        }
    } break;
    }

    return list_selector_event(level_picker->list_selector, event);
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

    const int selected_index = list_selector_selected(level_picker->list_selector);
    if (selected_index < 0) {
        return NULL;
    }

    const char **filenames = level_folder_filenames(level_picker->level_folder);

    return filenames[selected_index];
}

void level_picker_clean_selection(LevelPicker *level_picker)
{
    trace_assert(level_picker);
    list_selector_clean_selection(level_picker->list_selector);
}

int level_picker_enter_camera_event(LevelPicker *level_picker,
                                    Camera *camera)
{
    camera_center_at(camera, level_picker->camera_position);
    return 0;
}
