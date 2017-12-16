#include <stdio.h>
#include <assert.h>
#include <SDL2/SDL.h>

#include "./player.h"
#include "./platforms.h"
#include "./camera.h"
#include "./game.h"
#include "./error.h"

typedef enum game_state_t {
    GAME_STATE_RUNNING = 0,
    GAME_STATE_PAUSE,
    GAME_STATE_QUIT,

    GAME_STATE_N
} game_state_t;

typedef struct game_t {
    game_state_t state;
    player_t *player;
    platforms_t *platforms;
    camera_t *camera;
    char *level_file_path;
} game_t;

game_t *create_game(const char *level_file_path)
{
    assert(level_file_path);

    game_t *game = malloc(sizeof(game_t));
    if (game == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        goto malloc_game_failed;
    }

    if ((game->player = create_player(100.0f, 0.0f)) == NULL) {
        goto create_player_failed;
    }

    if ((game->platforms = load_platforms_from_file(level_file_path)) == NULL) {
        goto create_platforms_failed;
    }

    if ((game->camera = create_camera(vec(0.0f, 0.0f))) == NULL) {
        goto create_camera_failed;
    }

    if ((game->level_file_path = malloc(sizeof(char) * strlen(level_file_path))) == NULL) {
        goto malloc_level_file_name_failed;
    }

    strcpy(game->level_file_path, level_file_path);

    game->state = GAME_STATE_RUNNING;

    return game;

malloc_level_file_name_failed:
    free(game->camera);
create_camera_failed:
    free(game->platforms);
create_platforms_failed:
    free(game->player);
create_player_failed:
    free(game);
malloc_game_failed:
    return NULL;
}

void destroy_game(game_t *game)
{
    assert(game);

    if (game->level_file_path) { free(game->level_file_path); }
    if (game->camera) { free(game->camera); }
    if (game->platforms) { free(game->platforms); }
    if (game->player) { free(game->player); }

    free(game);
}

int game_render(const game_t *game, SDL_Renderer *renderer)
{
    assert(game);
    assert(renderer);

    if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    if (SDL_RenderClear(renderer) < 0) {
        throw_error(ERROR_TYPE_SDL2);
        return -1;
    }

    if (render_player(game->player, renderer, game->camera) < 0) {
        return -1;
    }

    if (render_platforms(game->platforms, renderer, game->camera) < 0) {
        return -1;
    }

    SDL_RenderPresent(renderer);

    return 0;
}

int game_update(game_t *game, Uint32 delta_time)
{
    assert(game);
    assert(delta_time > 0);

    update_player(game->player, game->platforms, delta_time);
    player_focus_camera(game->player, game->camera);

    return 0;
}

int game_event(game_t *game, const SDL_Event *event)
{
    assert(game);
    assert(event);

    switch (event->type) {
    case SDL_QUIT:
        game->state = GAME_STATE_QUIT;
        break;

    case SDL_KEYDOWN:
        switch (event->key.keysym.sym) {
        case SDLK_SPACE:
            player_jump(game->player);
            break;

        case SDLK_q:
            printf("Reloading the level from '%s'...", game->level_file_path);

            destroy_platforms(game->platforms);
            game->platforms = load_platforms_from_file(game->level_file_path);

            if (game->platforms == NULL) {
                print_current_error_msg("Could not reload the level");
                return -1;
            }
            break;
        }
        break;

    case SDL_JOYBUTTONDOWN:
        if (event->jbutton.button == 1) {
            player_jump(game->player);
        }
        break;
    }

    return 0;
}

int game_input(game_t *game,
               const Uint8 *const keyboard_state,
               SDL_Joystick *the_stick_of_joy)
{
    assert(game);
    assert(keyboard_state);
    assert(the_stick_of_joy);

    if (keyboard_state[SDL_SCANCODE_A]) {
        player_move_left(game->player);
    } else if (keyboard_state[SDL_SCANCODE_D]) {
        player_move_right(game->player);
    } else if (the_stick_of_joy && SDL_JoystickGetAxis(the_stick_of_joy, 0) < 0) {
        player_move_left(game->player);
    } else if (the_stick_of_joy && SDL_JoystickGetAxis(the_stick_of_joy, 0) > 0) {
        player_move_right(game->player);
    } else {
        player_stop(game->player);
    }

    return 0;
}

int is_game_over(const game_t *game)
{
    return game->state == GAME_STATE_QUIT;
}
