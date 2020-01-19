#ifndef LEVEL_PICKER_H_
#define LEVEL_PICKER_H_

#include <SDL.h>

#include "game/camera.h"
#include "game/level/background.h"
#include "ui/wiggly_text.h"
#include "dynarray.h"

typedef struct {
    Background background;
    Vec2f camera_position;
    WigglyText wiggly_text;
    Dynarray items;
    size_t items_cursor;
    int selected_item;
    Vec2f items_scroll;
    Vec2f items_position;
    Vec2f items_size;
} LevelPicker;

// TODO(#1221): Level Picker scroll does not support mouse wheel
// TODO(#1222): Level Picker scroll does not support dragging

void level_picker_populate(LevelPicker *level_picker,
                           const char *dirpath);

static inline
void destroy_level_picker(LevelPicker level_picker)
{
    free(level_picker.items.data);
}

int level_picker_render(const LevelPicker *level_picker,
                        const Camera *camera);
int level_picker_update(LevelPicker *level,
                        Camera *camera,
                        float delta_time);
int level_picker_event(LevelPicker *level_picker,
                       const SDL_Event *event);
int level_picker_input(LevelPicker *level_picker,
                       const Uint8 *const keyboard_state,
                       SDL_Joystick *the_stick_of_joy);
int level_picker_enter_camera_event(LevelPicker *level_picker,
                                    Camera *camera);
void level_picker_cursor_up(LevelPicker *level_picker);
void level_picker_cursor_down(LevelPicker *level_picker);

const char *level_picker_selected_level(const LevelPicker *level_picker);
void level_picker_clean_selection(LevelPicker *level_picker);


#endif  // LEVEL_PICKER_H_
