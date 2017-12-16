#include <stdio.h>
#include <assert.h>
#include <SDL2/SDL.h>

#include "./player.h"
#include "./platforms.h"
#include "./camera.h"
#include "./game.h"
#include "./error.h"

typedef struct game_t {
    player_t *player;
    platforms_t *platforms;
    camera_t *camera;
} game_t;

game_t *create_game(const char *platforms_file_path)
{
    assert(platforms_file_path);

    game_t *game = malloc(sizeof(game_t));
    if (game == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        goto malloc_game_failed;
    }

    if ((game->player = create_player(100.0f, 0.0f)) == NULL) {
        goto create_player_failed;
    }

    if ((game->platforms = load_platforms_from_file(platforms_file_path)) == NULL) {
        goto create_platforms_failed;
    }

    if ((game->camera = create_camera(vec(0.0f, 0.0f))) == NULL) {
        goto create_camera_failed;
    }

    return game;

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

    free(game->camera);
    free(game->platforms);
    free(game->player);
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

    return 0;
}

int game_event(game_t *game, const SDL_Event *event)
{
    assert(game);
    assert(event);

    return 0;
}

int game_input(game_t *game,
               const Uint8 *const keyboard_state,
               SDL_Joystick *the_stick_of_joy)
{
    assert(game);
    assert(keyboard_state);
    assert(the_stick_of_joy);

    return 0;
}
