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
#include "system/str.h"
#include "ebisp/builtins.h"
#include "broadcast.h"
#include "sdl/texture.h"
#include "game/level/level_editor/background_layer.h"
#include "game/level/level_editor.h"

static int game_render_cursor(const Game *game);

typedef enum Game_state {
    GAME_STATE_RUNNING = 0,
    GAME_STATE_PAUSE,
    GAME_STATE_CONSOLE,
    GAME_STATE_LEVEL_PICKER,
    GAME_STATE_LEVEL_EDITOR,
    GAME_STATE_QUIT
} Game_state;

typedef struct Game {
    Lt *lt;

    Game_state state;
    Broadcast *broadcast;
    Sprite_font *font;
    LevelPicker *level_picker;
    LevelEditor *level_editor;
    Level *level;
    Sound_samples *sound_samples;
    Camera *camera;
    Console *console;
    SDL_Renderer *renderer;
    SDL_Texture *texture_cursor;
    int cursor_x;
    int cursor_y;
} Game;

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

    game->state = GAME_STATE_LEVEL_PICKER;

    game->broadcast = PUSH_LT(
        lt,
        create_broadcast(game),
        destroy_broadcast);
    if (game->broadcast == NULL) {
        RETURN_LT(lt, NULL);
    }

    game->font = PUSH_LT(
        lt,
        create_sprite_font_from_file(
            "images/charmap-oldschool.bmp",
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

    game->sound_samples = PUSH_LT(
        lt,
        create_sound_samples(
            sound_sample_files,
            sound_sample_files_count),
        destroy_sound_samples);
    if (game->sound_samples == NULL) {
        RETURN_LT(lt, NULL);
    }

    game->camera = PUSH_LT(
        lt,
        create_camera(renderer, game->font),
        destroy_camera);
    if (game->camera == NULL) {
        RETURN_LT(lt, NULL);
    }

    game->console = PUSH_LT(
        lt,
        create_console(game->broadcast, game->font),
        destroy_console);
    if (game->console == NULL) {
        RETURN_LT(lt, NULL);
    }

    game->renderer = renderer;
    game->texture_cursor = PUSH_LT(
        lt,
        texture_from_bmp("images/cursor.bmp", renderer),
        SDL_DestroyTexture);
    if (SDL_SetTextureBlendMode(
            game->texture_cursor,
            SDL_ComposeCustomBlendMode(
                SDL_BLENDFACTOR_ONE_MINUS_DST_COLOR,
                SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR,
                SDL_BLENDOPERATION_ADD,
                SDL_BLENDFACTOR_ONE,
                SDL_BLENDFACTOR_ZERO,
                SDL_BLENDOPERATION_ADD)) < 0) {
        log_warn("SDL error: %s\n", SDL_GetError());
    }
    game->cursor_x = 0;
    game->cursor_y = 0;

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
    case GAME_STATE_RUNNING:
    case GAME_STATE_PAUSE: {
        if (level_render(game->level, game->camera) < 0) {
            return -1;
        }
    } break;

    case GAME_STATE_CONSOLE: {
        if (level_render(game->level, game->camera) < 0) {
            return -1;
        }

        if (console_render(game->console, game->camera, game->renderer) < 0) {
            return -1;
        }
    } break;

    case GAME_STATE_LEVEL_PICKER: {
        if (level_picker_render(game->level_picker, game->camera, game->renderer) < 0) {
            return -1;
        }

        if (game_render_cursor(game) < 0) {
            return -1;
        }
    } break;

    case GAME_STATE_LEVEL_EDITOR: {
        if (level_editor_render(game->level_editor, game->camera) < 0) {
            return -1;
        }

        if (game_render_cursor(game) < 0) {
            return -1;
        }
    } break;

    case GAME_STATE_QUIT: break;
    }

    return 0;
}

int game_sound(Game *game)
{
    switch (game->state) {
    case GAME_STATE_RUNNING:
    case GAME_STATE_PAUSE:
    case GAME_STATE_CONSOLE:
        return level_sound(game->level, game->sound_samples);
    case GAME_STATE_LEVEL_PICKER:
    case GAME_STATE_LEVEL_EDITOR:
    case GAME_STATE_QUIT:
        return 0;
    }

    return 0;
}

int game_update(Game *game, float delta_time)
{
    trace_assert(game);
    trace_assert(delta_time > 0.0f);

    switch (game->state) {
    case GAME_STATE_RUNNING: {
        if (level_update(game->level, delta_time) < 0) {
            return -1;
        }

        if (level_enter_camera_event(game->level, game->camera) < 0) {
            return -1;
        }

    } break;

    case GAME_STATE_CONSOLE: {
        if (level_update(game->level, delta_time) < 0) {
            return -1;
        }

        if (level_enter_camera_event(game->level, game->camera) < 0) {
            return -1;
        }

        if (console_update(game->console, delta_time) < 0) {
            return -1;
        }
    } break;

    case GAME_STATE_LEVEL_PICKER: {
        if (level_picker_update(game->level_picker, delta_time) < 0) {
            return -1;
        }

        if (level_picker_enter_camera_event(game->level_picker, game->camera) < 0) {
            return -1;
        }

        const char *level_filename = level_picker_selected_level(game->level_picker);

        if (level_filename != NULL) {
            if (game->level_editor == NULL) {
                game->level_editor = PUSH_LT(
                    game->lt,
                    create_level_editor_from_file(level_filename),
                    destroy_level_editor);
            } else {
                game->level_editor = RESET_LT(
                    game->lt,
                    game->level_editor,
                    create_level_editor_from_file(level_filename));
            }

            if (game->level_editor == NULL) {
                return -1;
            }

            if (game->level == NULL) {
                game->level = PUSH_LT(
                    game->lt,
                    create_level_from_level_editor(
                        game->level_editor,
                        game->broadcast),
                    destroy_level);
            } else {
                game->level = RESET_LT(
                    game->lt,
                    game->level,
                    create_level_from_level_editor(
                        game->level_editor,
                        game->broadcast));
            }

            if (game->level == NULL) {
                return -1;
            }

            game->state = GAME_STATE_RUNNING;
        }

    } break;

    case GAME_STATE_LEVEL_EDITOR: {
        if (level_editor_focus_camera(
                game->level_editor,
                game->camera) < 0) {
            return -1;
        }

        level_editor_update(game->level_editor, delta_time);
    } break;

    case GAME_STATE_PAUSE:
    case GAME_STATE_QUIT:
        break;
    }

    return 0;
}


static int game_event_pause(Game *game, const SDL_Event *event)
{
    trace_assert(game);
    trace_assert(event);

    switch (event->type) {
    case SDL_KEYDOWN:
        switch (event->key.keysym.sym) {
        case SDLK_p:
            game->state = GAME_STATE_RUNNING;
            camera_toggle_blackwhite_mode(game->camera);
            sound_samples_toggle_pause(game->sound_samples);
            break;
        case SDLK_l:
            camera_toggle_debug_mode(game->camera);
            level_toggle_debug_mode(game->level);
            break;
        }
        break;
    }

    return level_event(game->level, event);
}

static int game_event_running(Game *game, const SDL_Event *event)
{
    trace_assert(game);
    trace_assert(event);

    switch (event->type) {
    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_r: {
            const char *level_filename = game->level_editor->file_name;

            if (!level_filename) {
                log_warn("Could not reload the level. There is no associated file.\n");
                return 0;
            }

            log_info("Reloading the level from '%s'...\n", level_filename);

            game->level_editor = RESET_LT(
                game->lt,
                game->level_editor,
                create_level_editor_from_file(level_filename));
            if (game->level_editor == NULL) {
                log_fail("Could not reload level %s\n", level_filename);
                game->state = GAME_STATE_QUIT;
                return -1;
            }

            game->level = RESET_LT(
                game->lt,
                game->level,
                create_level_from_level_editor(
                    game->level_editor,
                    game->broadcast));
            if (game->level == NULL) {
                log_fail("Could not reload level %s\n", level_filename);
                game->state = GAME_STATE_QUIT;
                return -1;
            }

            camera_disable_debug_mode(game->camera);
        } break;

        case SDLK_p: {
            game->state = GAME_STATE_PAUSE;
            camera_toggle_blackwhite_mode(game->camera);
            sound_samples_toggle_pause(game->sound_samples);
        } break;

        case SDLK_l: {
            camera_toggle_debug_mode(game->camera);
            level_toggle_debug_mode(game->level);
        } break;

        case SDLK_TAB: {
            game->state = GAME_STATE_LEVEL_EDITOR;
        } break;
        }
    } break;

    case SDL_KEYUP: {
        switch (event->key.keysym.sym) {
        case SDLK_BACKQUOTE:
        case SDLK_c: {
            SDL_StartTextInput();
            game->state = GAME_STATE_CONSOLE;
            console_slide_down(game->console);
        } break;
        }
    } break;
    }

    return level_event(game->level, event);
}

static int game_event_console(Game *game, const SDL_Event *event)
{
    switch (event->type) {
    case SDL_KEYDOWN:
        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE:
            SDL_StopTextInput();
            game->state = GAME_STATE_RUNNING;
            return 0;

        default: {}
        }

    default: {}
    }

    return console_handle_event(game->console, event);
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
                    create_level_editor(),
                    destroy_level_editor);
            } else {
                game->level_editor = RESET_LT(
                    game->lt,
                    game->level_editor,
                    create_level_editor());
            }

            if (game->level_editor == NULL) {
                return -1;
            }

            if (game->level == NULL) {
                game->level = PUSH_LT(
                    game->lt,
                    create_level_from_level_editor(
                        game->level_editor,
                        game->broadcast),
                    destroy_level);
            } else {
                game->level = RESET_LT(
                    game->lt,
                    game->level,
                    create_level_from_level_editor(
                        game->level_editor,
                        game->broadcast));
            }

            if (game->level == NULL) {
                return -1;
            }

            game->state = GAME_STATE_RUNNING;
        } break;
        }
    } break;
    }

    return level_picker_event(game->level_picker, event, game->camera);
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
                    game->level_editor,
                    game->broadcast));
            if (game->level == NULL) {
                return -1;
            }
            game->state = GAME_STATE_RUNNING;
        } break;
        }
    } break;
    }

    return level_editor_event(game->level_editor, event, game->camera);
}

int game_event(Game *game, const SDL_Event *event)
{
    trace_assert(game);
    trace_assert(event);

    switch (event->type) {
    case SDL_QUIT: {
        game->state = GAME_STATE_QUIT;
        return 0;
    } break;

    case SDL_MOUSEMOTION: {
        game->cursor_x = event->motion.x;
        game->cursor_y = event->motion.y;
    } break;
    }

    switch (game->state) {
    case GAME_STATE_RUNNING:
        return game_event_running(game, event);

    case GAME_STATE_PAUSE:
        return game_event_pause(game, event);

    case GAME_STATE_CONSOLE:
        return game_event_console(game, event);

    case GAME_STATE_LEVEL_PICKER:
        return game_event_level_picker(game, event);

    case GAME_STATE_LEVEL_EDITOR:
        return game_event_level_editor(game, event);

    case GAME_STATE_QUIT:
        return 0;
    }

    return -1;
}


int game_input(Game *game,
               const Uint8 *const keyboard_state,
               SDL_Joystick *the_stick_of_joy)
{
    trace_assert(game);
    trace_assert(keyboard_state);

    switch (game->state) {
    case GAME_STATE_QUIT:
    case GAME_STATE_PAUSE:
    case GAME_STATE_CONSOLE:
    case GAME_STATE_LEVEL_EDITOR:
        return 0;

    case GAME_STATE_RUNNING:
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

struct EvalResult
game_send(Game *game, Gc *gc, struct Scope *scope,
          struct Expr path)
{
    trace_assert(game);
    trace_assert(gc);
    trace_assert(scope);

    const char *target = NULL;
    struct Expr rest = void_expr();
    struct EvalResult res = match_list(gc, "q*", path, &target, &rest);
    if (res.is_error) {
        return res;
    }

    if (strcmp(target, "level") == 0) {
        return level_send(game->level, gc, scope, rest);
    } else if (strcmp(target, "menu") == 0) {
        level_picker_clean_selection(game->level_picker);
        game->state = GAME_STATE_LEVEL_PICKER;
        return eval_success(NIL(gc));
    }

    return unknown_target(gc, "game", target);
}

// Private Functions

static int game_render_cursor(const Game *game)
{
    trace_assert(game);

    SDL_Rect src = {0, 0, 32, 32};
    SDL_Rect dest = {game->cursor_x, game->cursor_y, 32, 32};
    if (SDL_RenderCopy(game->renderer, game->texture_cursor, &src, &dest) < 0) {
        return -1;
    }

    return 0;
}
