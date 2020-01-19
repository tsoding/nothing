#include <SDL.h>
#include "system/stacktrace.h"
#include <stdio.h>

#include "game.h"
#include "game/level.h"
#include "game/sound_samples.h"
#include "game/level_picker.h"
#include "system/log.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "ui/console.h"
#include "ui/edit_field.h"
#include "ui/cursor.h"
#include "system/str.h"
#include "sdl/texture.h"
#include "game/level/level_editor/background_layer.h"
#include "game/level/level_editor.h"
#include "game/settings.h"
#include "game/credits.h"

typedef struct Game {
    Lt *lt;

    Game_state state;
    Sprite_font font;
    Memory level_editor_memory;
    LevelPicker level_picker;
    LevelEditor *level_editor;
    Credits credits;
    Level *level;
    Settings settings;
    Sound_samples *sound_samples;
    Camera camera;
    SDL_Renderer *renderer;
    Console *console;
    Cursor cursor;
    int console_enabled;
} Game;

void game_switch_state(Game *game, Game_state state)
{
    game->cursor.style = CURSOR_STYLE_POINTER;
    if (state == GAME_STATE_LEVEL_PICKER) {
        level_picker_clean_selection(&game->level_picker);
    }
    game->camera = create_camera(game->renderer, game->font);
    game->state = state;
}

Game *create_game(const char *level_folder,
                  const char *sound_sample_files[],
                  size_t sound_sample_files_count,
                  SDL_Renderer *renderer)
{
    trace_assert(level_folder);

    Lt *lt = create_lt();

    Game *game = PUSH_LT(lt, nth_calloc(1, sizeof(Game)), free);
    if (game == NULL) {
        RETURN_LT(lt, NULL);
    }
    game->lt = lt;

    game->font.texture = load_bmp_font_texture(
        renderer,
        "./assets/images/charmap-oldschool.bmp");

    game->level_editor_memory.capacity = LEVEL_EDITOR_MEMORY_CAPACITY;
    game->level_editor_memory.buffer = malloc(LEVEL_EDITOR_MEMORY_CAPACITY);
    trace_assert(game->level_editor_memory.buffer);

    level_picker_populate(&game->level_picker, level_folder);

    game->credits = create_credits();

    game->sound_samples = PUSH_LT(
        lt,
        create_sound_samples(
            sound_sample_files,
            sound_sample_files_count),
        destroy_sound_samples);
    if (game->sound_samples == NULL) {
        RETURN_LT(lt, NULL);
    }

    game->settings = create_settings();

    game->renderer = renderer;

    for (Cursor_Style style = 0; style < CURSOR_STYLE_N; ++style) {
        game->cursor.texs[style] = PUSH_LT(
            lt,
            texture_from_bmp(cursor_style_tex_files[style], renderer),
            SDL_DestroyTexture);
        if (SDL_SetTextureBlendMode(
                game->cursor.texs[style],
                SDL_ComposeCustomBlendMode(
                    SDL_BLENDFACTOR_ONE_MINUS_DST_COLOR,
                    SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR,
                    SDL_BLENDOPERATION_ADD,
                    SDL_BLENDFACTOR_ONE,
                    SDL_BLENDFACTOR_ZERO,
                    SDL_BLENDOPERATION_ADD)) < 0) {
            log_warn("SDL error while setting blending mode for `%s': %s\n",
                     cursor_style_tex_files[style],
                     SDL_GetError());
        }
    }

    game->level_editor = create_level_editor(
        &game->level_editor_memory,
        &game->cursor);

    game->console = PUSH_LT(
        lt,
        create_console(game),
        destroy_console);
    if (game->console == NULL) {
        RETURN_LT(lt, NULL);
    }
    game->console_enabled = 0;

    game_switch_state(game, GAME_STATE_LEVEL_PICKER);

    return game;
}

void destroy_game(Game *game)
{
    trace_assert(game);
    destroy_level_picker(game->level_picker);
    free(game->level_editor_memory.buffer);
    RETURN_LT0(game->lt);
}

int game_render(const Game *game)
{
    trace_assert(game);

    switch(game->state) {
    case GAME_STATE_LEVEL: {
        if (level_render(game->level, &game->camera) < 0) {
            return -1;
        }
    } break;

    case GAME_STATE_LEVEL_PICKER: {
        if (level_picker_render(&game->level_picker, &game->camera) < 0) {
            return -1;
        }
    } break;

    case GAME_STATE_LEVEL_EDITOR: {
        if (level_editor_render(game->level_editor, &game->camera) < 0) {
            return -1;
        }
    } break;

    case GAME_STATE_CREDITS: {
        if (credits_render(&game->credits, &game->camera) < 0) {
            return -1;
        }
    } break;

    case GAME_STATE_SETTINGS: {
        settings_render(&game->settings, &game->camera);
    } break;

    case GAME_STATE_QUIT: break;
    }

    if (game->console_enabled) {
        if (console_render(game->console, &game->camera) < 0) {
            return -1;
        }
    }

    if (cursor_render(&game->cursor, game->renderer) < 0) {
        return -1;
    }

    return 0;
}

int game_sound(Game *game)
{
    switch (game->state) {
    case GAME_STATE_LEVEL:
        return level_sound(game->level, game->sound_samples);
    case GAME_STATE_LEVEL_EDITOR:
        level_editor_sound(game->level_editor, game->sound_samples);
        return 0;
    case GAME_STATE_LEVEL_PICKER:
    case GAME_STATE_CREDITS:
    case GAME_STATE_SETTINGS:
    case GAME_STATE_QUIT:
        return 0;
    }

    return 0;
}

int game_update(Game *game, float delta_time)
{
    trace_assert(game);
    trace_assert(delta_time > 0.0f);

    // TODO(#1218): effective scale recalculation should be probably done only when the size of the window is changed
    SDL_Rect view_port;
    SDL_RenderGetViewport(game->camera.renderer, &view_port);
    game->camera.effective_scale = effective_scale(&view_port);

    if (game->console_enabled) {
        if (console_update(game->console, delta_time) < 0) {
            return -1;
        }
    }

    switch (game->state) {
    case GAME_STATE_LEVEL: {
        if (level_update(game->level, delta_time) < 0) {
            return -1;
        }

        if (level_enter_camera_event(game->level, &game->camera) < 0) {
            return -1;
        }

    } break;

    case GAME_STATE_LEVEL_PICKER: {
        if (level_picker_update(&game->level_picker, &game->camera, delta_time) < 0) {
            return -1;
        }

        if (level_picker_enter_camera_event(&game->level_picker, &game->camera) < 0) {
            return -1;
        }

        const char *level_filename = level_picker_selected_level(&game->level_picker);

        if (level_filename != NULL) {
            if (game_load_level(game, level_filename) < 0) {
                return -1;
            }
        }
    } break;

    case GAME_STATE_LEVEL_EDITOR: {
        if (level_editor_focus_camera(
                game->level_editor,
                &game->camera) < 0) {
            return -1;
        }

        level_editor_update(game->level_editor, delta_time);
    } break;

    case GAME_STATE_CREDITS: {
        if (credits_update(&game->credits, &game->camera, delta_time) < 0) {
            return -1;
        }
    } break;

    case GAME_STATE_SETTINGS: {
        settings_update(&game->settings, &game->camera, delta_time);
        sound_samples_update_volume(
            game->sound_samples,
            game->settings.volume_slider.value);
    } break;

    case GAME_STATE_QUIT:
        break;
    }

    return 0;
}

static int game_event_running(Game *game, const SDL_Event *event)
{
    trace_assert(game);
    trace_assert(event);

    if (!SDL_IsTextInputActive()) {
        switch (event->type) {
        case SDL_KEYDOWN: {
            switch (event->key.keysym.sym) {
            case SDLK_r: {
                game->level = RESET_LT(
                    game->lt,
                    game->level,
                    create_level_from_level_editor(
                        game->level_editor));
                if (game->level == NULL) {
                    game_switch_state(game, GAME_STATE_QUIT);
                    return -1;
                }

                level_disable_pause_mode(
                    game->level,
                    &game->camera,
                    game->sound_samples);
                camera_disable_debug_mode(&game->camera);
            } break;

            case SDLK_TAB: {
                game_switch_state(game, GAME_STATE_LEVEL_EDITOR);
            } break;
            }
        } break;
        }
    }

    return level_event(game->level, event, &game->camera, game->sound_samples);
}

static int game_event_level_picker(Game *game, const SDL_Event *event)
{
    trace_assert(game);
    trace_assert(event);

    switch (event->type) {
    case SDL_KEYDOWN: {
        switch(event->key.keysym.sym) {
        case SDLK_UP: {
            level_picker_cursor_up(&game->level_picker);
        } break;

        case SDLK_DOWN: {
            level_picker_cursor_down(&game->level_picker);
        } break;

        case SDLK_p: {
            if (event->key.keysym.mod & KMOD_CTRL) {
                level_picker_cursor_up(&game->level_picker);
            }
        } break;

        case SDLK_n:  {
            if (event->key.keysym.mod & KMOD_CTRL) {
                level_picker_cursor_down(&game->level_picker);
            } else {
                memory_clean(&game->level_editor_memory);
                game->level_editor = create_level_editor(
                    &game->level_editor_memory,
                    &game->cursor);

                if (game->level == NULL) {
                    game->level = PUSH_LT(
                        game->lt,
                        create_level_from_level_editor(
                            game->level_editor),
                        destroy_level);
                } else {
                    game->level = RESET_LT(
                        game->lt,
                        game->level,
                        create_level_from_level_editor(
                            game->level_editor));
                }

                if (game->level == NULL) {
                    return -1;
                }

                game_switch_state(game, GAME_STATE_LEVEL);
            }
        } break;

        case SDLK_i: {
            game_switch_state(game, GAME_STATE_CREDITS);
        } break;

        case SDLK_s: {
            game_switch_state(game, GAME_STATE_SETTINGS);
        } break;
        }
    } break;
    }

    return level_picker_event(&game->level_picker, event);
}

static int game_event_level_editor(Game *game, const SDL_Event *event)
{
    trace_assert(game);
    trace_assert(event);

    switch (event->type) {
    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_TAB: {
            game->level = RESET_LT(
                game->lt,
                game->level,
                create_level_from_level_editor(
                    game->level_editor));
            if (game->level == NULL) {
                return -1;
            }
            game_switch_state(game, GAME_STATE_LEVEL);
        } break;
        }
    } break;
    }

    return level_editor_event(game->level_editor, event, &game->camera, &game->level_editor_memory);
}

int game_event(Game *game, const SDL_Event *event)
{
    trace_assert(game);
    trace_assert(event);

    // Global event handling
    switch (event->type) {
    case SDL_QUIT: {
        game_switch_state(game, GAME_STATE_QUIT);
        return 0;
    } break;

    case SDL_KEYDOWN: {
        if ((event->key.keysym.sym == SDLK_q && event->key.keysym.mod & KMOD_CTRL) ||
            (event->key.keysym.sym == SDLK_F4 && event->key.keysym.mod & KMOD_ALT)) {
            game_switch_state(game, GAME_STATE_QUIT);
            return 0;
        }
    } break;
    }

    // Console event handling
    if (game->console_enabled) {
        switch (event->type) {
        case SDL_KEYDOWN:
            switch (event->key.keysym.sym) {
            case SDLK_ESCAPE:
                SDL_StopTextInput();
                game->console_enabled = 0;
                return 0;
            default: {}
            }

        default: {}
        }

        return console_handle_event(game->console, event);
    } else {
        switch (event->type) {
        case SDL_KEYUP: {
            switch (event->key.keysym.sym) {
            case SDLK_BACKQUOTE:
            case SDLK_c: {
                if (event->key.keysym.mod == KMOD_NONE || event->key.keysym.mod == KMOD_NUM) {
                    SDL_StartTextInput();
                    game->console_enabled = 1;
                    console_slide_down(game->console);
                }
            } break;
            }
        } break;
        }
    }

    // State event handling
    switch (game->state) {
    case GAME_STATE_LEVEL:
        return game_event_running(game, event);

    case GAME_STATE_LEVEL_PICKER:
        return game_event_level_picker(game, event);

    case GAME_STATE_LEVEL_EDITOR:
        return game_event_level_editor(game, event);

    case GAME_STATE_CREDITS: {
        switch (event->type) {
        case SDL_KEYDOWN: {
            if (event->key.keysym.sym == SDLK_ESCAPE) {
                game_switch_state(game, GAME_STATE_LEVEL_PICKER);
                return 0;
            }
        } break;
        }

        return 0;
    } break;

    case GAME_STATE_SETTINGS: {
        switch (event->type) {
        case SDL_KEYDOWN: {
            if (event->key.keysym.sym == SDLK_ESCAPE) {
                game_switch_state(game, GAME_STATE_LEVEL_PICKER);
                return 0;
            }
        } break;
        }

        settings_event(&game->settings, &game->camera, event);
        return 0;
    } break;

    case GAME_STATE_QUIT:
        return 0;
    }

    return -1;
}

// TODO(#1145): get rid of keyboard_state and introduce *_joystick methods
//
// keyboard_state is a global var and can be check anywhere anyway
int game_input(Game *game,
               const Uint8 *const keyboard_state,
               SDL_Joystick *the_stick_of_joy)
{
    trace_assert(game);
    trace_assert(keyboard_state);

    if (game->console_enabled) {
        return 0;
    }

    switch (game->state) {
    case GAME_STATE_SETTINGS:
    case GAME_STATE_CREDITS:
    case GAME_STATE_QUIT:
    case GAME_STATE_LEVEL_EDITOR:
        return 0;

    case GAME_STATE_LEVEL:
        return level_input(game->level, keyboard_state, the_stick_of_joy);

    case GAME_STATE_LEVEL_PICKER:
        return level_picker_input(&game->level_picker, keyboard_state, the_stick_of_joy);
    }

    return 0;
}

int game_over_check(const Game *game)
{
    return game->state == GAME_STATE_QUIT;
}

int game_load_level(Game *game, const char *level_filename)
{
    trace_assert(game);
    trace_assert(level_filename);

    memory_clean(&game->level_editor_memory);
    game->level_editor =
        create_level_editor_from_file(
            &game->level_editor_memory,
            &game->cursor,
            level_filename);

    if (!game->level_editor) {
        game_switch_state(game, GAME_STATE_LEVEL_PICKER);
        return 0;
    }

    if (game->level == NULL) {
        game->level = PUSH_LT(
            game->lt,
            create_level_from_level_editor(
                game->level_editor),
            destroy_level);
    } else {
        game->level = RESET_LT(
            game->lt,
            game->level,
            create_level_from_level_editor(
                game->level_editor));
    }

    if (game->level == NULL) {
        return -1;
    }

    game_switch_state(game, GAME_STATE_LEVEL);

    return 0;
}
