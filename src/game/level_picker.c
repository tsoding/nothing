#include <stdio.h>

#include "./level_picker.h"
#include "system/stacktrace.h"
#include "system/lt.h"
#include "system/nth_alloc.h"

struct LevelPicker
{
    Lt *lt;
};

LevelPicker *create_level_picker(const char *dirpath)
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

    return level_picker;
}

void destroy_level_picker(LevelPicker *level_picker)
{
    trace_assert(level_picker);
    RETURN_LT0(level_picker->lt);
}

int level_picker_render(const LevelPicker *level_picker,
                        SDL_Renderer *renderer)
{
    trace_assert(level_picker);
    trace_assert(renderer);
    return 0;
}

int level_picker_update(LevelPicker *level,
                        float delta_time)
{
    trace_assert(level);
    (void) delta_time;
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
    trace_assert(the_stick_of_joy);
    return 0;
}

const char *level_picker_selected_level(const LevelPicker *level_picker)
{
    trace_assert(level_picker);
    return NULL;
}

void level_picker_clean_selection(LevelPicker *level_picker)
{
    trace_assert(level_picker);
}
