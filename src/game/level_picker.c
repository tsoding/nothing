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

/* TODO(#606): LevelPicker doesn't allow to select any levels */

struct LevelPicker
{
    Lt *lt;
    const char *dirpath;
    Background *background;
    Vec camera_position;
    ListSelector *list_selector;
};

LevelPicker *create_level_picker(const Sprite_font *sprite_font, const char *dirpath)
{
    trace_assert(dirpath);

    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    LevelPicker *level_picker = PUSH_LT(
        lt,
        nth_alloc(sizeof(LevelPicker)),
        free);
    if (level_picker == NULL) {
        RETURN_LT(lt, NULL);
    }
    level_picker->lt = lt;

    level_picker->dirpath = PUSH_LT(
        lt,
        string_duplicate(dirpath, NULL),
        free);
    if (level_picker->dirpath == NULL) {
        RETURN_LT(lt, NULL);
    }

    level_picker->background = PUSH_LT(
        lt,
        create_background(hexstr("073642")),
        destroy_background);
    if (level_picker->background == NULL) {
        RETURN_LT(lt, NULL);
    }

    level_picker->camera_position = vec(0.0f, 0.0f);

    const char *items[] = {
        "Hello",
        "World",
        "Khooy"
    };
    const size_t items_count = sizeof(items) / sizeof(const char *);

    // TODO: ListSelector is not centered in LevelPicker
    level_picker->list_selector = PUSH_LT(
        lt,
        create_list_selector(
            sprite_font,
            items,
            items_count,
            vec(50.0f, 50.0f),
            100.0f),
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

    if (list_selector_render(level_picker->list_selector, renderer)) {
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

    return 0;
}

int level_picker_event(LevelPicker *level_picker, const SDL_Event *event)
{
    trace_assert(level_picker);
    trace_assert(event);
    return 0;
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
    return NULL; //level_picker->dirpath;
}

void level_picker_clean_selection(LevelPicker *level_picker)
{
    trace_assert(level_picker);
}

int level_picker_enter_camera_event(LevelPicker *level_picker,
                                    Camera *camera)
{
    camera_center_at(camera, level_picker->camera_position);
    return 0;
}
