#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <assert.h>
#include <stdio.h>

#include "game.h"
#include "game/level.h"
#include "game/sound_samples.h"
#include "system/error.h"
#include "system/log.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "ui/console.h"
#include "ui/edit_field.h"

typedef enum Game_state {
    GAME_STATE_RUNNING = 0,
    GAME_STATE_PAUSE,
    GAME_STATE_CONSOLE,
    GAME_STATE_QUIT,

    GAME_STATE_N
} Game_state;

typedef struct Game {
    Lt *lt;

    Game_state state;
    Level *level;
    char *level_file_path;
    Sound_samples *sound_samples;
    Camera *camera;
    Sprite_font *font;
    Console *console;
    SDL_Renderer *renderer;
} Game;

Game *create_game(const char *level_file_path,
                    const char *sound_sample_files[],
                    size_t sound_sample_files_count,
                    SDL_Renderer *renderer)
{
    assert(level_file_path);

    Lt *const lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    Game *game = PUSH_LT(lt, nth_alloc(sizeof(Game)), free);
    if (game == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    game->lt = lt;

    game->renderer = renderer;

    game->level = PUSH_LT(
        lt,
        create_level_from_file(level_file_path),
        destroy_level);
    if (game->level == NULL) {
        RETURN_LT(lt, NULL);
    }

    game->level_file_path = PUSH_LT(lt, nth_alloc(sizeof(char) * (strlen(level_file_path) + 1)), free);
    if (game->level_file_path == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    strcpy(game->level_file_path, level_file_path);

    game->font = PUSH_LT(
        lt,
        create_sprite_font_from_file("fonts/charmap-oldschool.bmp", renderer),
        destroy_sprite_font);
    if (game->font == NULL) {
        RETURN_LT(lt, NULL);
    }

    game->camera = PUSH_LT(lt, create_camera(renderer, game->font), destroy_camera);
    if (game->camera == NULL) {
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

    game->console = PUSH_LT(
        lt,
        create_console(game->level, game->font),
        destroy_console);
    if (game->console == NULL) {
        RETURN_LT(lt, NULL);
    }

    game->state = GAME_STATE_RUNNING;

    return game;
}

void destroy_game(Game *game)
{
    assert(game);
    RETURN_LT0(game->lt);
}

int game_render(const Game *game)
{
    assert(game);

    if (game->state == GAME_STATE_QUIT) {
        return 0;
    }

    if (level_render(game->level, game->camera) < 0) {
        return -1;
    }

    if (game->state == GAME_STATE_CONSOLE) {
        if (console_render(game->console, game->renderer) < 0) {
            return -1;
        }
    }

    return 0;
}

int game_sound(Game *game)
{
    return level_sound(game->level, game->sound_samples);
}

int game_update(Game *game, float delta_time)
{
    assert(game);
    assert(delta_time > 0.0f);

    if (game->state == GAME_STATE_QUIT) {
        return 0;
    }

    if (game->state == GAME_STATE_RUNNING || game->state == GAME_STATE_CONSOLE) {
        if (level_update(game->level, delta_time) < 0) {
            return -1;
        }

        if (level_enter_camera_event(game->level, game->camera) < 0) {
            return -1;
        }

        if (console_update(game->console, delta_time) < 0) {
            return -1;
        }
    }

    return 0;
}


static int game_event_pause(Game *game, const SDL_Event *event)
{
    assert(game);
    assert(event);

    switch (event->type) {
    case SDL_QUIT:
        game->state = GAME_STATE_QUIT;
        break;

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
    assert(game);
    assert(event);

    switch (event->type) {
    case SDL_QUIT:
        game->state = GAME_STATE_QUIT;
        break;

    case SDL_KEYDOWN:
        switch (event->key.keysym.sym) {
        case SDLK_r:
            log_info("Reloading the level from '%s'...\n", game->level_file_path);

            game->level = RESET_LT(
                game->lt,
                game->level,
                create_level_from_file(
                    game->level_file_path));

            if (game->level == NULL) {
                print_current_error_msg("Could not reload the level");
                game->state = GAME_STATE_QUIT;
                return -1;
            }

            camera_disable_debug_mode(game->camera);

            break;

        case SDLK_q:
            log_info("Reloading the level's platforms from '%s'...\n", game->level_file_path);
            if (level_reload_preserve_player(game->level, game->level_file_path) < 0) {
                print_current_error_msg("Could not reload the level");
                game->state = GAME_STATE_QUIT;
                return -1;
            }
            break;

        case SDLK_p:
            game->state = GAME_STATE_PAUSE;
            camera_toggle_blackwhite_mode(game->camera);
            sound_samples_toggle_pause(game->sound_samples);
            break;

        case SDLK_l:
            camera_toggle_debug_mode(game->camera);
            level_toggle_debug_mode(game->level);
            break;
        }
        break;
    case SDL_KEYUP:
        switch (event->key.keysym.sym) {
        case SDLK_BACKQUOTE:
        case SDLK_c:
            SDL_StartTextInput();
            game->state = GAME_STATE_CONSOLE;
            console_slide_down(game->console);
            break;
        }
        break;

    }

    return level_event(game->level, event);
}

static int game_event_console(Game *game, const SDL_Event *event)
{
    switch (event->type) {
    case SDL_QUIT:
        game->state = GAME_STATE_QUIT;
        return 0;

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

int game_event(Game *game, const SDL_Event *event)
{
    assert(game);
    assert(event);

    switch (game->state) {
    case GAME_STATE_RUNNING:
        return game_event_running(game, event);

    case GAME_STATE_PAUSE:
        return game_event_pause(game, event);

    case GAME_STATE_CONSOLE:
        return game_event_console(game, event);

    default: {}
    }

    return 0;
}


int game_input(Game *game,
               const Uint8 *const keyboard_state,
               SDL_Joystick *the_stick_of_joy)
{
    assert(game);
    assert(keyboard_state);

    if (game->state == GAME_STATE_QUIT  ||
        game->state == GAME_STATE_PAUSE ||
        game->state == GAME_STATE_CONSOLE) {
        return 0;
    }

    return level_input(game->level, keyboard_state, the_stick_of_joy);
}

int game_over_check(const Game *game)
{
    return game->state == GAME_STATE_QUIT;
}
