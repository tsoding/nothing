#include <assert.h>
#include <SDL2/SDL.h>

#include "./lt.h"
#include "./game/level/player.h"
#include "./game/level/platforms.h"
#include "./game/level.h"
#include "./game/level/camera.h"
#include "./error.h"
#include "./goals.h"
#include "./lava.h"

/* TODO(#118): Level doesn't play the Nothing and Something sounds when the goal get into view */

struct level_t
{
    lt_t *lt;
    player_t *player;
    platforms_t *platforms;
    camera_t *camera;
    goals_t *goals;
    lava_t *lava;
    color_t background_color;
};

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

    char color[7];
    if (fscanf(level_file, "%6s", color) == EOF) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    level->background_color = color_from_hexstr(color);

    level->player = PUSH_LT(lt, create_player_from_stream(level_file), destroy_player);
    if (level->player == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->platforms = PUSH_LT(lt, create_platforms_from_stream(level_file), destroy_platforms);
    if (level->platforms == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->goals = PUSH_LT(lt, create_goals_from_stream(level_file), destroy_goals);
    if (level->goals == NULL) {
        RETURN_LT(lt, NULL);
    }

    level->lava = PUSH_LT(lt, create_lava_from_stream(level_file), destroy_lava);
    if (level->lava == NULL) {
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

    if (camera_clear_background(level->camera, renderer, level->background_color) < 0) {
        return -1;
    }

    if (player_render(level->player, renderer, level->camera) < 0) {
        return -1;
    }

    if (lava_render(level->lava, renderer, level->camera) < 0) {
        return -1;
    }

    if (platforms_render(level->platforms, renderer, level->camera) < 0) {
        return -1;
    }

    if (goals_render(level->goals, renderer, level->camera) < 0) {
        return -1;
    }

    /* TODO(#157): goals_cue is not supposed to be invoked in level_render
     *
     * But I simply couldn't find a better place for it.
     */
    goals_cue(level->goals, renderer, level->camera);

    return 0;
}

int level_update(level_t *level, float delta_time)
{
    assert(level);
    assert(delta_time > 0);

    player_update(level->player, level->platforms, delta_time);
    player_focus_camera(level->player, level->camera);
    player_hide_goals(level->player, level->goals);
    player_die_from_lava(level->player, level->lava);

    goals_update(level->goals, delta_time);
    goals_checkpoint(level->goals, level->player);
    lava_update(level->lava, delta_time);

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

void level_toggle_debug_mode(level_t *level)
{
    camera_toggle_debug_mode(level->camera);
}

void level_toggle_pause_mode(level_t *level)
{
    camera_toggle_blackwhite_mode(level->camera);
}

int level_reload_preserve_player(level_t *level, const char *file_name)
{
    lt_t * const lt = create_lt();
    if (lt == NULL) {
        return -1;
    }

    /* TODO(#104): duplicate code in create_level_from_file and level_reload_preserve_player */

    FILE * const level_file = PUSH_LT(lt, fopen(file_name, "r"), fclose);
    if (level_file == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, -1);
    }

    char color[7];
    if (fscanf(level_file, "%6s", color) == EOF) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, -1);
    }
    level->background_color = color_from_hexstr(color);

    player_t * const skipped_player = create_player_from_stream(level_file);
    if (skipped_player == NULL) {
        RETURN_LT(lt, -1);
    }
    destroy_player(skipped_player);

    platforms_t * const platforms = create_platforms_from_stream(level_file);
    if (platforms == NULL) {
        RETURN_LT(lt, -1);
    }
    level->platforms = RESET_LT(level->lt, level->platforms, platforms);

    goals_t * const goals = create_goals_from_stream(level_file);
    if (goals == NULL) {
        RETURN_LT(lt, -1);
    }
    level->goals = RESET_LT(level->lt, level->goals, goals);

    lava_t * const lava = create_lava_from_stream(level_file);
    if (lava == NULL) {
        RETURN_LT(lt, -1);
    }

    RETURN_LT(lt, 0);
}

int level_sound(level_t *level, sound_medium_t *sound_medium)
{
    if (goals_sound(level->goals, sound_medium) < 0) {
        return -1;
    }

    return 0;
}
