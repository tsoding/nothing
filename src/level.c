#include <assert.h>
#include <SDL2/SDL.h>

#include "./lt.h"
#include "./player.h"
#include "./platforms.h"
#include "./level.h"
#include "./camera.h"
#include "./error.h"

struct level_t
{
    lt_t *lt;
    player_t *player;
    platforms_t *platforms;
    camera_t *camera;
};

level_t *create_level(player_t *player,
                      platforms_t *platforms,
                      camera_t *camera)
{
    assert(player);
    assert(platforms);

    lt_t *const lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    level_t *const level = PUSH_LT(lt, malloc(sizeof(level_t)), free);
    if (level == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    level->player = PUSH_LT(lt, player, destroy_player);
    level->platforms = PUSH_LT(lt, platforms, destroy_platforms);
    level->camera = PUSH_LT(lt, camera, destroy_camera);
    level->lt = lt;

    return level;
}

level_t *create_level_from_file(const char *file_name)
{
    assert(file_name);

    lt_t *const lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    level_t *const level = PUSH_LT(lt, malloc(sizeof(level_t)), free);
    if (level == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    FILE *level_file = PUSH_LT(lt, fopen(file_name, "r"), fclose);
    if (level_file == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    level->player = PUSH_LT(lt, create_player_from_stream(level_file), destroy_player);
    if (level->player == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->platforms = PUSH_LT(lt, create_platforms_from_stream(level_file), destroy_platforms);
    if (level->platforms == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->camera = PUSH_LT(lt, create_camera(vec(0.0f, 0.0f)), destroy_camera);
    if (level->camera == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->lt = lt;

    fclose(RELEASE_LT(lt, level_file));

    return level;
}

void destroy_level(level_t *level)
{
    assert(level);
    RETURN_LT0(level->lt);
}

int level_render(const level_t *level, SDL_Renderer *renderer)
{
    assert(level);
    assert(renderer);

    if (render_player(level->player, renderer, level->camera) < 0) {
        return -1;
    }

    if (render_platforms(level->platforms, renderer, level->camera) < 0) {
        return -1;
    }

    return 0;
}

int level_update(level_t *level, Uint32 delta_time)
{
    assert(level);
    assert(delta_time > 0);

    update_player(level->player, level->platforms, delta_time);
    player_focus_camera(level->player, level->camera);

    return 0;
}

int level_event(level_t *level, const SDL_Event *event)
{
    assert(level);
    assert(event);

    switch (event->type) {
    case SDL_KEYDOWN:
        switch (event->key.keysym.sym) {
        case SDLK_SPACE:
            player_jump(level->player);
            break;
        }
        break;

    case SDL_JOYBUTTONDOWN:
        if (event->jbutton.button == 1) {
            player_jump(level->player);
        }
        break;
    }

    return 0;
}

int level_input(level_t *level,
                const Uint8 *const keyboard_state,
                SDL_Joystick *the_stick_of_joy)
{
    assert(level);
    assert(keyboard_state);
    (void) the_stick_of_joy;

    if (keyboard_state[SDL_SCANCODE_A]) {
        player_move_left(level->player);
    } else if (keyboard_state[SDL_SCANCODE_D]) {
        player_move_right(level->player);
    } else if (the_stick_of_joy && SDL_JoystickGetAxis(the_stick_of_joy, 0) < 0) {
        player_move_left(level->player);
    } else if (the_stick_of_joy && SDL_JoystickGetAxis(the_stick_of_joy, 0) > 0) {
        player_move_right(level->player);
    } else {
        player_stop(level->player);
    }

    return 0;
}
