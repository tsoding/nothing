#include <SDL.h>
#include "system/stacktrace.h"
#include <stdio.h>

#include "game.h"
#include "game/level.h"
#include "game/sound_samples.h"
#include "game/level_picker.h"
#include "game/credits.h"
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

typedef enum Game_state {
    GAME_STATE_LEVEL = 0,
    GAME_STATE_LEVEL_PICKER,
    GAME_STATE_LEVEL_EDITOR,
    GAME_STATE_SETTINGS,
    GAME_STATE_CREDITS,
    GAME_STATE_QUIT
} Game_state;

typedef struct Game {
    Lt *lt;

    Game_state state;
    Sprite_font *font;
    LevelPicker *level_picker;
    LevelEditor *level_editor;
    Level *level;
    Settings settings;
    Credits *credits;
    Sound_samples *sound_samples;
    Camera camera;
    SDL_Renderer *renderer;
    Console *console;
    Cursor cursor;
    int console_enabled;
} Game;

static
void game_switch_state(Game *game, Game_state state)
{
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

    game->font = PUSH_LT(
        lt,
        create_sprite_font_from_file(
            "./assets/images/charmap-oldschool.bmp",
            renderer),
        destroy_sprite_font);
    if (game->font == NULL) {
        RETURN_LT(lt, NULL);
    }

    game->level_picker = PUSH_LT(
        lt,
        create_level_picker(
            game->font,
            level_folder),
        destroy_level_picker);
    if (game->level_picker == NULL) {
        RETURN_LT(lt, NULL);
    }

    game->credits = PUSH_LT(
        lt,
        create_credits(
            game->font),
        destroy_credits);
    if (game->credits == NULL) {
        RETURN_LT(lt, NULL);
    }

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
            log_warn("SDL error: %s\n", SDL_GetError());
        }
    }


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
        if (level_picker_render(game->level_picker, &game->camera) < 0) {
            return -1;
        }
    } break;

    case GAME_STATE_CREDITS: {
        if (credits_render(game->credits, &game->camera) < 0) {
            return -1;
        }
    } break;

    case GAME_STATE_LEVEL_EDITOR: {
        if (level_editor_render(game->level_editor, &game->camera) < 0) {
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
    case GAME_STATE_SETTINGS:
    case GAME_STATE_CREDITS:
    case GAME_STATE_QUIT:
        return 0;
    }

    return 0;
}

int game_update(Game *game, float delta_time)
{
    trace_assert(game);
    trace_assert(delta_time > 0.0f);

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
        if (level_picker_update(game->level_picker, delta_time) < 0) {
            return -1;
        }

        if (level_picker_enter_camera_event(game->level_picker, &game->camera) < 0) {
            return -1;
        }

        const char *level_filename = level_picker_selected_level(game->level_picker);

        if (level_filename != NULL) {
            if (game_load_level(game, level_filename) < 0) {
                return -1;
            }
        }
    } break;

    case GAME_STATE_CREDITS: {
        if (credits_update(game->credits, delta_time) < 0) {
            return -1;
        }
        if (credits_enter_camera_event(game->credits, &game->camera) < 0) {
            return -1;
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
        case SDLK_n: {
            if (game->level_editor == NULL) {
                game->level_editor = PUSH_LT(
                    game->lt,
                    create_level_editor(&game->cursor),
                    destroy_level_editor);
            } else {
                game->level_editor = RESET_LT(
                    game->lt,
                    game->level_editor,
                    create_level_editor(&game->cursor));
            }

            if (game->level_editor == NULL) {
                return -1;
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
        } break;

        case SDLK_s: {
            game_switch_state(game, GAME_STATE_SETTINGS);
        } break;

        case SDLK_i: {
            game_switch_state(game, GAME_STATE_CREDITS);
        } break;
        }
    } break;
    }

    return level_picker_event(game->level_picker, event, &game->camera);
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

    return level_editor_event(game->level_editor, event, &game->camera);
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
        if (event->key.keysym.sym == SDLK_q && event->key.keysym.mod & KMOD_CTRL) {
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
                SDL_StartTextInput();
                game->console_enabled = 1;
                console_slide_down(game->console);
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

    case GAME_STATE_CREDITS: {
        switch (event->type) {
        case SDL_KEYDOWN: {
            if (event->key.keysym.sym == SDLK_ESCAPE) {
                game_switch_state(game, GAME_STATE_LEVEL_PICKER);
                return 0;
            }
        } break;
        }

        credits_event(&game->credits, &game->camera, event);
        return 0;
    } break;

    case GAME_STATE_QUIT:
        return 0;
    }

    return -1;
}


// TODO: get rid of keyboard_state (because it's a global var and can
// be check anywhere anyway). And introduce *_joystick methods.
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
        return level_picker_input(game->level_picker, keyboard_state, the_stick_of_joy);
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

    if (game->level_editor == NULL) {
        game->level_editor = PUSH_LT(
            game->lt,
            create_level_editor_from_file(level_filename, &game->cursor),
            destroy_level_editor);
    } else {
        game->level_editor = RESET_LT(
            game->lt,
            game->level_editor,
            create_level_editor_from_file(level_filename, &game->cursor));
    }

    if (game->level_editor == NULL) {
        return -1;
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
