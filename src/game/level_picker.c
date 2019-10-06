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
    Background *background;
    Vec2f camera_position;
    LevelFolder *level_folder;
    WigglyText wiggly_text;
    Slider volume_slider;
    Vec2f volume_slider_scale;
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

    level_picker->wiggly_text = (WigglyText) {
        .text = "Select Level",
        .scale = {10.0f, 10.0f},
        .color = COLOR_WHITE,
    };

    level_picker->volume_slider = (Slider) {
                                     .drag = 0,
                                     .value = 80.0f,
                                     .max_value = 100.0f,
    };
    level_picker->volume_slider_scale = vec(0.25f, 0.10f);

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
                        const Camera *camera)
{
    trace_assert(level_picker);

    const Rect viewport = camera_view_port_screen(camera);

    if (background_render(level_picker->background, camera) < 0) {
        return -1;
    }

    const Vec2f title_size = wiggly_text_size(&level_picker->wiggly_text, camera);

    if (wiggly_text_render(
            &level_picker->wiggly_text,
            camera,
            vec(viewport.w * 0.5f - title_size.x * 0.5f, TITLE_MARGIN_TOP)) < 0) {
        return -1;
    }

    if (list_selector_render(level_picker->list_selector, camera->renderer) < 0) {
        return -1;
    }

    {
        /* CSS */
        const float padding = 20.0f;
        const Vec2f size = vec(3.0f, 3.0f);
        const Vec2f position = vec(0.0f, viewport.h - size.y * FONT_CHAR_HEIGHT);

        /* HTML */
        if (camera_render_text_screen(
                camera,
                "Press 'N' to create new level",
                size,
                COLOR_WHITE,
                vec(position.x + padding,
                    position.y - padding)) < 0) {
            return -1;
        }
    }

    {
        /* CSS volume */
        const Rect position = {
          .w = viewport.w * level_picker->volume_slider_scale.x,
          .h = viewport.h * level_picker->volume_slider_scale.y,
          .x = viewport.w - viewport.w * level_picker->volume_slider_scale.x - 5.0f,
          .y = 5.0f,
        };

        /* HTML volume */
        if (slider_render(&level_picker->volume_slider, camera, position) < 0) {
            return -1;
        }
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
                       const SDL_Event *event,
                       const Camera *camera)
{
    trace_assert(level_picker);
    trace_assert(event);

    {
      const Rect viewport = camera_view_port_screen(camera);
      const Rect position = {
                             .w = viewport.w * level_picker->volume_slider_scale.x,
                             .h = viewport.h * level_picker->volume_slider_scale.y,
                             .x = viewport.w - viewport.w * level_picker->volume_slider_scale.x - 5.0f,
                             .y = 5.0f,
      };
      int selected = 0;
      if (slider_event(
                &level_picker->volume_slider,
                event,
                position,
                &selected) < 0) {
        return -1;
      }
      if(selected){
        return 0;
      }
    }
    switch (event->type) {
    case SDL_WINDOWEVENT: {
        switch (event->window.event) {
        case SDL_WINDOWEVENT_SHOWN:
        case SDL_WINDOWEVENT_RESIZED: {
            const Vec2f font_scale = vec(5.0f, 5.0f);
            const float padding_bottom = 50.0f;

            int width;
            SDL_GetWindowSize(SDL_GetWindowFromID(event->window.windowID), &width, NULL);

            const Vec2f title_size = wiggly_text_size(&level_picker->wiggly_text, camera);

            const Vec2f selector_size = list_selector_size(
                level_picker->list_selector,
                font_scale,
                padding_bottom);

            list_selector_move(
                level_picker->list_selector,
                vec((float)width * 0.5f - selector_size.x * 0.5f,
                    TITLE_MARGIN_TOP + title_size.y + TITLE_MARGIN_BOTTOM));
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

float level_picker_get_volume(LevelPicker *level_picker)
{
  trace_assert(level_picker);
  return level_picker->volume_slider.value;
}
